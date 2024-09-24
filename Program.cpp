#include <filesystem>
#include "Program.h"
#include <iostream>

namespace WixInstaller
{
    public ref class Program
    {
        static int main(array<System::String^>^ args)
        {
            array<WixSharp::Dir^>^ verDirs = gcnew array<WixSharp::Dir^>(1);
            array<WixSharp::Files^>^ verFiles = gcnew array<WixSharp::Files^>(1);

            std::cout << std::filesystem::current_path();
            WixSharp::File^ theFile = gcnew File("..\\big_file_32M.msi");

            ManagedProject^ project = gcnew ManagedProject("BIMAL",
                                        gcnew Dir("C:\\MyDocs\\cygwin64",
                                          theFile
                                                        ));

            WixSharp::Dir^ dir21 = gcnew Dir("C:\\MyDocs\\cygwin64\\etc");
            WixSharp::Dir^ dir24 = gcnew InstallDir("C:\\Users\\AppData\\Roaming\\Autodesk\\Revit\Addins\\2024");
            WixSharp::Dir^ uniFile = gcnew Dir("RevitAddin\\Files\\BIMAL.addin");
            WixSharp::Dir^ hoste = gcnew Dir("hosts");

            auto fullPath24 = WixSharp::CommonTasks::Tasks::AddDir(dir24, uniFile);
            verDirs[0] = fullPath24;            


            
//            System::Collections::Generic::List<String^>^ aWxsFiles = gcnew System::Collections::Generic::List<String^>;
//            aWxsFiles->Add("C:\\MyDocs\\cygwin64\\etc\\hosts");
//            project->WxsFiles = aWxsFiles;

            project->Dirs = verDirs;

            Console::WriteLine("Listing project files...");
            for each (WixSharp::File^ f in project->AllFiles)
            {
                Console::WriteLine(f->Name);
            }

            //project.ResolveWildCards().FindFile(f => f.Name.EndsWith("ExportToApplication.exe")).First()
            //    .Shortcuts = new[] { new FileShortcut("ExportTo", "%Desktop%") };
            project->Scope = WixSharp::InstallScope::perUser;
            project->OutFileName = "BIMAL Installer";
            project->ControlPanelInfo->Manufacturer = "BIMAL";
           
            System::Guid ^Guido = gcnew Guid("DCE38760-AC90-4A3F-BDCF-CE8876E9D5B0");           
            String^ msiGUID = "DCE38760-AC90-4A3F-BDCF-CE8876E9D5B0";

//            project->&GUID = Guido;
//            project->UpgradeCode = Guido;

            project->Version = gcnew Version("2.1.1.4"); // Last number always update, if even old version

            MajorUpgrade^ projectMajorUpgrade = gcnew MajorUpgrade();
            projectMajorUpgrade->AllowSameVersionUpgrades = true;
            projectMajorUpgrade->Schedule = UpgradeSchedule::afterInstallInitialize;

            projectMajorUpgrade->DowngradeErrorMessage = "You have newer version of BIMAL.";

            project->ManagedUI = gcnew ManagedUI();            
            project->ManagedUI->InstallDialogs->Add(Dialogs::Welcome);
            project->ManagedUI->InstallDialogs->Add(Dialogs::Licence);
            project->ManagedUI->InstallDialogs->Add(Dialogs::Progress);
            project->ManagedUI->InstallDialogs->Add(Dialogs::Exit);

            project->ManagedUI->ModifyDialogs->Add(Dialogs::MaintenanceType);
            project->ManagedUI->ModifyDialogs->Add(Dialogs::Features);
            project->ManagedUI->ModifyDialogs->Add(Dialogs::Progress);
            project->ManagedUI->ModifyDialogs->Add(Dialogs::Exit);
            
            project->UIInitialized += gcnew ManagedProject::SetupEventHandler(&UI_Initialized);
            
            project->BeforeInstall += gcnew ManagedProject::SetupEventHandler(&MSI_BeforeInstall);

            project->Load += gcnew ManagedProject::SetupEventHandler(&MSI_Load);

            project->AfterInstall += gcnew ManagedProject::SetupEventHandler(&MSI_AfterInstall);

           
            project->BuildMsi("C:\\Users\\Documents\\installer.wix\\wix\\installer.wix.msi");

            return 0;
        }

        static void UI_Initialized(SetupEventArgs^ e)
        {
            Version^ installedVersion = WixSharp::Extensions::LookupInstalledVersion(e->Session);
            Version^ thisVersion = WixSharp::Extensions::QueryProductVersion(e->Session);

            if (installedVersion != nullptr)
            {
                if (thisVersion <= installedVersion)
                {
                    e->ManagedUI->Shell->ErrorDetected = true;
                    e->ManagedUI->Shell->CustomErrorDescription = "You have newer version of BIMAL.\nVersion: " + installedVersion;
                    e->Result = ActionResult::UserExit;
                }
                else
                {
                    e->ManagedUI->Shell->Dialogs->Remove(Dialogs::Welcome);
                    e->ManagedUI->Shell->Dialogs->Remove(Dialogs::Licence);
                    e->ManagedUI->Shell->Dialogs->Insert(0, Dialogs::Features);
                }
            }
        }

        static void MSI_BeforeInstall(SetupEventArgs^ e)
        {
            if (e->IsInstalling)
            {                
                String^ pathToFile = System::IO::Path::Combine(e->InstallDir + "BackgroundService\net8.0\win-x64\MTS_BackgroundService.exe");
                Microsoft::Win32::RegistryKey^ key = Microsoft::Win32::Registry::CurrentUser->OpenSubKey("SOFTWARE\Microsoft\Windows\CurrentVersion\Run", true);
                key->SetValue("BIMAL BackgroundService", pathToFile);
            }
        }

        static void MSI_Load(SetupEventArgs^ e)
        {
            if (e->IsUninstalling)
            {
                array<Process^>^ processes = Process::GetProcessesByName("MTS_BackgroundService");

                if (processes->Length > 0)
                {                    
                    for each (Process^ process in processes)
                    {
                        process->Kill();
                        process->WaitForExit();
                    }
                }
            }
        }

        static void MSI_AfterInstall(SetupEventArgs^ e)
        {
            if (e->IsInstalling)
            {
                String^ pathToFile = System::IO::Path::Combine(e->InstallDir + "BackgroundService\net8.0\win-x64\MTS_BackgroundService.exe");
                Process::Start(pathToFile);
            }

            if (e->IsUninstalling)
            {
                Microsoft::Win32::RegistryKey^ key = Microsoft::Win32::Registry::CurrentUser->OpenSubKey("SOFTWARE\Microsoft\Windows\CurrentVersion\Run", true);
                key->DeleteValue("BIMAL BackgroundService", false);
            }
        }
    };
}
#include"fileMenu.h"
// return true when the user hits the OK button or the cancel button 
// file_name will be empty (size == 0) if they hit ceanel
//

// this file list will only be loaded when changing directories or when opening the file menu
// it should start empty and only  be loaded when empty
static std::vector<std::string> file_list = {};
static std::string curr_dir = "";

bool fileMenu(std::string &file_name){

    // if the file list is empty then populate it
    // read whatever the current dir is and store all the file names
    if (file_list.size() == 0){
        if(curr_dir.size() ==0){
            // reserve a size of 1024 characters for the current directory
            // TODO: handle allocation failure
            char * dir_ptr = new char [1024];
            dir_ptr[1023] = 0;
            curr_dir = getcwd(dir_ptr, 1023);

            delete [] dir_ptr;
        }

        // populate file list
        DIR * dir = opendir(curr_dir.c_str());

        struct dirent *files_dir = readdir(dir);
        while(files_dir != NULL){
            if (strcmp(files_dir->d_name, ".") != 0){
                file_list.push_back(files_dir->d_name);
            }
            files_dir = readdir(dir);

        }
        closedir(dir);
    }

    static std::string curr_file;
    // get all files in current directory
    // print them as selectables
    // only show images (png, jpg, jpeg, TIFF, BMP)
    // if the OK button is pressed, 
    ImGui::Begin("file menu");
    ImGui::Text(curr_dir.c_str());
    static int32_t selected = 0;

    //list all the files in the current dir
    bool reload_files = false;
    int32_t i = -1, double_clicked = -1;
    // list of files in current folder
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
    if(ImGui::BeginChild("file list",ImVec2(0, 400), true, window_flags )){

        for (auto x : file_list){
            i+=1;
            if(ImGui::Selectable(x.c_str(), i == selected)){
                selected = i;
            }
            if (ImGui::IsMouseDoubleClicked(0) 
                    && ImGui::IsItemActive()
                    && ImGui::IsItemHovered()){
                reload_files = true;
                double_clicked = i;
                }
            }
        

        if (reload_files){
            // go into the selected location if it is a directory
            if (double_clicked < 0){
                std::cerr<< "no directory selected\n";
            }
            std::string new_dir = curr_dir;
            new_dir.append("/");
            new_dir.append(file_list[double_clicked]);

            if (chdir(new_dir.c_str()) != 0){
                std::cerr << "error openinig: " << new_dir<< "\n";
            }
            else{
                // repopulate file list
                char * dir_ptr = new char [1024];
                dir_ptr[1023] = 0;
                curr_dir = getcwd(dir_ptr, 1023);

                delete [] dir_ptr;

                // populate file list
                DIR * dir = opendir(curr_dir.c_str());

                struct dirent *files_dir = readdir(dir);
                file_list.clear();
                while(files_dir != NULL){
                    if (strcmp(files_dir->d_name, ".") != 0){
                        file_list.push_back(files_dir->d_name);
                    }
                    files_dir = readdir(dir);
                }
                closedir(dir);
            }
        }
        ImGui::EndChild();
    }

    if (selected > 0){
    std::string curr_file = curr_dir + file_list[selected];
    ImGui::Text(curr_file.c_str());
            }
    bool exit_ok = false, exit_cancel = false;
    // go into that folder if you double click the folder
    if (ImGui::Button("OK")) exit_ok = true;

    
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) exit_cancel = true;
    

    ImGui::End();

    if (exit_cancel){
        file_name.clear();
        file_list.clear();
        return true;
    }

    if (exit_ok){
        if (selected <=0){
            std::cerr << "file selection not valid\n";
            file_name.clear();
            return true;
        }
        file_name = curr_dir;
        file_name.append("/");
        file_name.append(file_list[selected]);
        file_list.clear();

    }

    return false;
}


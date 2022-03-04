#include "imageMenu.h"

// keep a list of images in the database (selectables that preview when clicked)
//
std::vector<GLuint> tex_ids(0);
   
bool imageMenu(std::vector<image> &image_list) {

    static bool open_file_menu = false;
    static std::string new_img = "";

    ImGui::Text("image list");
    ImGui::SameLine();
    if(ImGui::Button("add image")){

        open_file_menu = true;
    }
    ImGui::Separator();
    int32_t i = -1;
    static int32_t selected = -1;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
    ImGui::BeginChild("img list", ImVec2(0, 0), true, window_flags);
    if (image_list.size() == 0){
        ImGui::Text("No images were found!");
    }


    // if the image_list has changed size, then regen and rebind all the textures

    if (image_list.size() != tex_ids.size()){
        glDeleteTextures(tex_ids.size(), tex_ids.data());
        tex_ids.resize(image_list.size());
        glGenTextures(tex_ids.size(), tex_ids.data());
    }
    for (auto x: image_list){
        i+=1;

        ImGui::Text(x.name.c_str());

        // convert image to a reasonable size
        // max of one dimenstion should be 200
        double largest_side = (x.datamat.rows > x.datamat.cols) ? x.datamat.rows : x.datamat.cols;
        double img_scale = (double)200/largest_side;
        cv::Mat scaled_img;
        cv::resize(x.datamat, scaled_img, cv::Size(), img_scale, img_scale, cv::INTER_CUBIC);

        // run through the conversions to display all the images
        cv::Mat disp_img(scaled_img.size(), scaled_img.type());
        // BGR -> RGB   
        // R  2 -> 0
        // G 1 -> 1
        // B 0 -> 2
        int from_to[] = { 2,0,1,1,0,2};
        cv::mixChannels(&scaled_img, 1, &disp_img, 1, from_to, 3);
        
        glBindTexture(GL_TEXTURE_2D, tex_ids[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexImage2D(GL_TEXTURE_2D, 
                0, 
                GL_RGB8, 
                disp_img.cols, 
                disp_img.rows, 
                0,
                GL_RGB, 
                GL_UNSIGNED_BYTE, 
                disp_img.data);

        if(ImGui::ImageButton((void *)(intptr_t)tex_ids[i],
                     ImVec2(disp_img.cols, disp_img.rows))){
            selected = i;
        }
    }

    ImGui::EndChild();

    // open the file menu here
    if (open_file_menu){
        open_file_menu = !(fileMenu(new_img));
        // if the string size is > 0, then there is a file to load.
        if (new_img.size() > 0){
            // load the image
            // open the filel with opencv
            std::cerr << new_img << "\n";
            cv::Mat orig_image = cv::imread(new_img.c_str(), cv::IMREAD_COLOR);
            auto img_arr = cv::InputArray(orig_image);
            // use jpg compression (it's fast)
            std::vector<unsigned char> img_out = {};
            // lossless jpg compression
            cv::imencode(".jpg", img_arr, img_out, {cv::IMWRITE_JPEG_QUALITY, 100});

            // pass the image buffer and the filename to be saved in the database.
            // adding the +1 excludes the '/'
            size_t last_slash_i =  new_img.rfind("/") + 1; 
            std::string img_name = new_img.substr(last_slash_i, new_img.size() - last_slash_i);
            std::cerr << img_name<< "\n";
            saveImg(img_out, img_name);

            open_file_menu = false;

            new_img.clear();

            // return true if you need to reload the song list from the database
            return true;
        }
    }
    


    return false;
}


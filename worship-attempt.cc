#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Native_File_Chooser.h>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_Text_Display.H>

static std::vector<Fl_Image*> im_vec;

class PresWin : public Fl_Window {
    public:
        Fl_Image *cur_img = NULL, *next_img = NULL;
        std::string cur_txt = "", next_txt = "";
    
    PresWin(int x, int y, int w, int h, const char *label = NULL) : Fl_Window(x, y, w, h, label) {

    }

    void draw() {
        cur_img->draw(0, 0);
    }
};

static PresWin pres_win(0, 0, 0, 0, "Pres window");

void show_full_win(Fl_Widget *w, void*){
    pres_win.show();
    pres_win.cur_img = im_vec[0];
}

int main(int argc, char *argv[]){
    pres_win.fullscreen();
    im_vec.push_back(Fl_PNG_Image("./worship extreme main menu.png").copy());

    Fl_Window editor_window(720, 720, "Editor Window");
    Fl_Button present(160, 210, 80, 40, "Present");
    present.callback(show_full_win);


    editor_window.show();
    return Fl::run();
}
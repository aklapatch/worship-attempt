#include <string>
#include <iostream>
#include <cstdlib>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Native_File_Chooser.h>
#include <FL/Fl_Output.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Display.H>

static Fl_Window full_win(0, 0, "Presenter Window");

void present_cb(Fl_Widget *w, void*) { 
    full_win.show();
}

int main(int argc, char *argv[]){
    full_win.fullscreen();

    Fl_Window editor_window(720, 720, "Editor Window");
    Fl_Button present(160, 210, 80, 40, "Present");
    present.callback(present_cb);
    editor_window.show();
    return Fl::run();
}
#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Scroll.H>
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
#include <FL/Fl_Plugin.H>
#include <FL/Fl_Pixmap.H>
#include <FL/platform.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Color_Chooser.H>

static std::vector<Fl_Image*> im_vec;

// The editing interface will look a little like Google slides.
// With the list of slides on the left and a preview more in the center.
// Different songs can be selected with a dropdown choose menu

#define TXT_MARGIN (10)

class PreviewButton : public Fl_Button {
    public:
        Fl_Fontsize font_size = 50; 
        uchar r = 0, g = 0, b = 0;
        Fl_Font font = FL_HELVETICA;
        Fl_Image *full_img = NULL, *scaled_img = NULL;
        std::string txt = "";

    PreviewButton(int x, int y, int w, int h, Fl_Image *img = NULL) : 
        Fl_Button(x, y, w, h, "") {

        // keep a reference of the full size image
        full_img = img;
        if (img != NULL){
            scaled_img = img->copy(this->w(), this->h());
        }
    }
    void draw() {
        if (scaled_img != NULL && (scaled_img->w() != this->w() || scaled_img->h() != this->h())){
            scaled_img = NULL;
        }
        if (scaled_img == NULL && full_img != NULL){
            scaled_img = full_img->copy(this->w(), this->h());
        }
        if (scaled_img != NULL){
            scaled_img->draw(this->x(), this->y(), this->w(), this->h());
        }

        fl_color(r, g, b);
        fl_font(font, font_size);
        // try to put the text at the center of the screen
        int tmp_w, tmp_h;
        fl_measure(txt.c_str(), tmp_w, tmp_h);
        int txt_x = this->x() + this->w()/2 - (tmp_w/2), txt_y = this->y() + (this->h()/2) - (tmp_h/2);
        fl_draw(txt.c_str(), txt_x, txt_y, tmp_w, tmp_h, FL_ALIGN_CENTER, NULL, 0);

        // draw a rectangle to show that this is a button
        fl_rect(this->x(), this->y(), this->w(), this->h());
    }
};

// TODO: add function to push content to presWin, then make the pre_button callback call that function.
class EditSlide: public Fl_Group {
    public:
        PreviewButton im_button;
        Fl_Button pres_button;

    EditSlide(int x, int y, int w, const char *label = "edit slide", Fl_Image* in_img=NULL) :
        // The 1 here needs to stay a 1. Tried 0 and 
        // The image button didn't come up
        Fl_Group(x, y, w, 1, label),
        im_button(x, y, w, 0, in_img),
        pres_button(x, y, w, 0, "Show") {
        end();

        im_button.full_img = in_img;

        int txt_x, txt_y, txt_w, txt_h;
        fl_font(pres_button.labelfont(), pres_button.labelsize());
        fl_text_extents("Show", txt_x, txt_y, txt_w, txt_h);
        txt_w += TXT_MARGIN, txt_h += TXT_MARGIN;
        // keep the w over txt_w
        w = txt_w > w ? txt_w : w;

        //adjust the image button to be a 16/9 aspect ratio
        int ratio_h = (w*9)/16;
        // doing this resize after resizing the widgets messes things up.
        this->size(w, ratio_h + txt_h);

        im_button.size(w, ratio_h);
        pres_button.resize(x, y + ratio_h, w, txt_h);
    }
};

struct Slides {
    Fl_Font font = FL_HELVETICA;
    Fl_Fontsize font_size = 50;
    std::vector<std::string> lyrics = {};
    Fl_Image *full_size_im = NULL;
    std::string background_name = "";
};

static Slides edit_slides;

struct Set {
    std::vector<Slides> slides = {};
};

class PresWin : public Fl_Window {
    public:
        Fl_Fontsize font_size = 50; 
        uchar r = 0, g = 0, b = 0;
        Fl_Font font = FL_HELVETICA;
        Fl_Image *cur_img = NULL, *next_img = NULL;
        std::string cur_txt = "", next_txt = "";
    
    PresWin(int x, int y, int w, int h, const char *label = NULL) : Fl_Window(x, y, w, h, label) {}

    void draw() {
        if (cur_img != NULL){
            cur_img->draw(this->x(), this->y(), this->w(), this->h());
        }

        fl_color(r, g, b);
        fl_font(font, font_size);
        // try to put the text at the center of the screen
        int tmp_w, tmp_h;
        fl_measure(cur_txt.c_str(), tmp_w, tmp_h);
        int txt_x = this->x_root() + this->w()/2 - (tmp_w/2), txt_y = this->y_root() + (this->h()/2) - (tmp_h/2);
        fl_draw(cur_txt.c_str(), txt_x, txt_y, tmp_w, tmp_h, FL_ALIGN_CENTER, NULL, 0);
    }
};

static PresWin pres_win(0, 0, 0, 0, "Pres window");

#define STR(x) #x

Fl_Window editor_window(720, 720, "Editor Window");
Fl_Button present(300, 210, 80, 40, "Present");
Fl_Input_Choice font_in(100, 100, 200, 20, "font number");
Fl_Int_Input font_size_in(300, 300, 50, 30, "font size");
Fl_Input text_in(300, 510, 200, 30, "text in");
Fl_Color_Chooser color_in(400, 50, 200, 100, "font color");
Fl_Button add_slide_b(200, 150, 25, 25, "+");
Fl_Scroll slide_edit_list(50, 200, 0, 0, "Slides");

#define MAP_FONT(x) STR(x), x
std::map<std::string, Fl_Font> font_val_map = {
    {MAP_FONT(FL_HELVETICA)},
    {MAP_FONT(FL_HELVETICA_BOLD)},
    {MAP_FONT(FL_HELVETICA_ITALIC)},
    {MAP_FONT(FL_COURIER)},
    {MAP_FONT(FL_COURIER_BOLD)},
    {MAP_FONT(FL_COURIER_ITALIC)},
    {MAP_FONT(FL_COURIER_BOLD_ITALIC)},
    {MAP_FONT(FL_TIMES)},
    {MAP_FONT(FL_TIMES_BOLD)},
    {MAP_FONT(FL_TIMES_ITALIC)},
    {MAP_FONT(FL_TIMES_BOLD_ITALIC)},
};

#define TOCHAR(x) (uchar)((x)*255.0)
void show_full_win(Fl_Widget *w, void *)
{
    pres_win.r = TOCHAR(color_in.r());
    pres_win.g = TOCHAR(color_in.g());
    pres_win.b = TOCHAR(color_in.b());
    pres_win.cur_txt = text_in.value();
    pres_win.cur_img = im_vec[0];
    pres_win.font_size = atoi(font_size_in.value());
    pres_win.font = font_val_map[font_in.value()];
    pres_win.show();
}

// TODO: bring up edit window
void edit_cb(Fl_Widget *w, void*){
    std::cout << "stes";
}

// TODO: have the present buttons push content to the main window.
void show_cb(Fl_Widget *w, void*){
    // grab the image from the parent and set it
    EditSlide *parent = (EditSlide*)w->parent();
    if (parent == NULL){
        std::cout << "null parent " << __LINE__ << "\n";
        return;
    }
    pres_win.font = parent->im_button.font;
    pres_win.font_size = parent->im_button.font_size;
    pres_win.cur_txt = parent->im_button.txt;
    pres_win.cur_img = parent->im_button.full_img;
    pres_win.show();
    pres_win.redraw();
}

int main(int argc, char *argv[]){
    editor_window.end();
    pres_win.fullscreen();
    im_vec.push_back(Fl_PNG_Image("./worship extreme main menu.png").copy());

    for (auto x: font_val_map){
        font_in.add(x.first.c_str());
    }
    font_in.value(STR(FL_HELVETICA));
    font_size_in.value("50");
    // set font color to rgb mode
    color_in.mode(0);
    color_in.rgb(1, 1, 1);
    text_in.value("test label");
    present.callback(show_full_win);
    EditSlide slide(slide_edit_list.x(), slide_edit_list.y(), 300, "edit slide", im_vec[0]);
    slide.pres_button.callback(show_cb);
    slide_edit_list.add(slide);
    slide_edit_list.size(slide.w() + slide_edit_list.scrollbar_size(), slide.h() + slide_edit_list.scrollbar_size());

    editor_window.show();
    slide_edit_list.scroll_to(0, 0);
    slide_edit_list.redraw();
    return Fl::run();
}
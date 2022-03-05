use fltk::{app, button::Button, frame::Frame, prelude::*, window::Window, image::*, enums };

#[derive(Debug, Clone, Copy)]
pub enum Message{ Show,
    ChangePic(usize),
}

fn main() {
    let app = app::App::default().with_scheme(app::Scheme::Gleam);
    let mut wind = Window::new(100, 100, 400, 300, "Editor window");

    let mut frame = Frame::new(0, 0, 400, 200, "");
    let mut but = Button::new(160, 210, 80, 40, "open window");
    wind.make_resizable(true);
    wind.clear_modal_states();
    wind.end();
    wind.show();

    let mut wind1 = Window::new(0, 0, 400, 300, "display_window");
    wind1.clear_modal_states();
    wind1.fullscreen(true);

    let mut img_frame : Frame = Frame::default().size_of(&wind1);
    wind1.end();
    wind1.hide();

    let img_arr = [ SharedImage::load("./worship extreme main menu.png").expect("Failed"), SharedImage::load("./sunset.jpg").expect("failed") ];
    let mut img_i: usize = 0;
    let mut img_arr_len = img_arr.len();

    let (s, r) = app::channel::<Message>();

    but.set_callback( move |_| s.send(Message::Show));
    //s.send(Message::Show));

    img_frame.handle( move |_, ev| match ev {
            enums::Event::KeyUp => {
            img_i += 1;
            if img_i >= img_arr_len {
                img_i = 0;
            }
            if app::event_key() == enums::Key::Right {
                s.send(Message::ChangePic(img_i));
            }
            true
        }
        _ => false,
    });

    while app.wait() {
        if let Some(msg) = r.recv() {
            match msg {
                Message::Show => {
                    wind1.show();
                    img_frame.set_size(wind1.pixel_w(), wind1.pixel_h());
                    img_frame.set_pos(wind1.x(), wind1.y());
                    img_frame.set_image_scaled(Some(img_arr[0].copy()));
                    img_frame.redraw();
                },
                Message::ChangePic(pic_i) => {
                    img_frame.set_image_scaled(Some(img_arr[pic_i].copy()));
                    img_frame.redraw();
                    wind1.redraw();
                },
            }
        }
    }
}


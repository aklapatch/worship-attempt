use fltk::{app, button::Button, frame::Frame, prelude::*, window::Window};

fn main() {
    let app = app::App::default().with_scheme(app::Scheme::Gleam);
    let mut wind = Window::new(100, 100, 400, 300, "Editor window");

    let mut frame = Frame::new(0, 0, 400, 200, "");
    let mut but = Button::new(160, 210, 80, 40, "open window");
    wind.make_resizable(true);
    wind.end();
    wind.show();

    let mut wind1 = Window::default()
        .with_label("test2");
    wind1.fullscreen(true);
    wind1.clear_modal_states();
    wind1.end();
    wind1.hide();

    but.set_callback(move |_| wind1.show());

    app.run().unwrap();
}


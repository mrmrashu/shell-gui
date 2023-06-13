#include <gtk/gtk.h>

void on_window_closed(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

void on_button_clicked(GtkButton *button, gpointer data)
{
    GtkWidget *window = GTK_WIDGET(data);
    gtk_window_close(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *button;
    GtkWidget *headerbar;
    GtkWidget *textbox;
    GtkWidget *submitbutton;

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Shell 0.1 ~ Project Semicolon");

    // Create a custom header bar
    headerbar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(headerbar), "Shell 0.1 ~ Project Semicolon");

    // Load CSS file
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "style.css", NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(headerbar);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Set up the custom CSS class for the GtkEntry widget
    const gchar *class_name = "entry-style";
    GtkStyleContext *context_entry = gtk_widget_get_style_context(window);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_class(context_entry, class_name);

    // Set the custom header bar as the title bar of the window
    gtk_window_set_titlebar(GTK_WINDOW(window), headerbar);

    // Parent Container Box window
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    // I/O Container Box
    GtkWidget *IObox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_end(GTK_BOX(box), IObox, FALSE, FALSE,0);

    // Submit Command Button
    submitbutton = gtk_button_new_with_label("Submit");
    gtk_widget_set_name(submitbutton, "submitbutton");
    //Submit Button Placement
    gtk_box_pack_end(GTK_BOX(IObox), submitbutton, FALSE, FALSE,0);

    textbox = gtk_entry_new();
    gtk_widget_set_name(textbox, "commandbox");
    gtk_style_context_add_class(gtk_widget_get_style_context(textbox), "commandbox"); // Add a CSS class to the widget

    gtk_entry_set_width_chars(GTK_ENTRY(textbox), 30); // Set the width to fit 30 characters
    gtk_box_pack_end(GTK_BOX(IObox), textbox, TRUE, TRUE, 0);

    GtkWidget *box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(headerbar), box2);

    // Exit Session Button
    button = gtk_button_new_with_label("");
    gtk_widget_set_name(button, "close-button");
    gtk_style_context_add_class(gtk_widget_get_style_context(textbox), "close-button"); // Add a CSS class to the widget

    gtk_box_pack_start(GTK_BOX(box2), button, FALSE, FALSE, 0);

    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), window);

    // Connect the "destroy" signal to the window close event
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_closed), NULL);

    gtk_window_set_default_size(GTK_WINDOW(window), 700, 300);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

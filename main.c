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
    GtkWidget *box;

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Shell 0.1 ~ Project Semicolon");
    // Create a custom header bar
    GtkWidget *headerbar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(headerbar), "Shell 0.1 ~ Project Semicolon");
    // gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(headerbar), FALSE);

    // Load CSS file
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "style.css", NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(headerbar);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Set the custom header bar as the title bar of the window
    gtk_window_set_titlebar(GTK_WINDOW(window), headerbar);

    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    button = gtk_button_new_with_label("Exit Session");
    gtk_widget_set_name(button, "close-button");
    gtk_box_pack_end(GTK_BOX(box), button, TRUE, TRUE, 0);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(headerbar), box);

    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), window);


    // Connect the "destroy" signal to the window close event
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_closed), NULL);

    // Add other widgets and functionality to the window

    gtk_window_set_default_size(GTK_WINDOW(window), 700, 300);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

#include <gtk/gtk.h>

void on_window_closed(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

void on_entry_activate(GtkEntry *entry, gpointer data) {
    const gchar *text = gtk_entry_get_text(entry);
    GtkLabel *label = GTK_LABEL(data);
    gtk_label_set_text(label, text);
    // Clear the text from the entry
    gtk_entry_set_text(GTK_ENTRY(entry), "");
}

void on_button_clicked(GtkButton *button, gpointer data)
{
    GtkWidget *window = GTK_WIDGET(data);
    gtk_window_close(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Application widgets
    GtkWidget *button;
    GtkWidget *headerbar;
    GtkWidget *textbox;
    GtkWidget *display;
    GtkWidget *submitbutton;

    // Main Window (Entry)
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Shell 0.1 ~ Project Semicolon");

    // Headerbar
    headerbar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(headerbar), "Shell 0.1 ~ Project Semicolon");
    
    // Setting the custom header bar as the title bar of the window
    gtk_window_set_titlebar(GTK_WINDOW(window), headerbar);

    // Loading Stylesheets 
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "style.css", NULL);
    
    // Style Context for overriding default styles
    GtkStyleContext *context = gtk_widget_get_style_context(headerbar);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Custom CSS class for the GtkEntry widget
    const gchar *class_name = "entry-style";
    GtkStyleContext *context_entry = gtk_widget_get_style_context(window);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_class(context_entry, class_name);

    /*
        Containership
        window -> headerbar -> headerbox -> box.end ->
        window -> box -> box.end -> entry
    */

    // Parent Container Box window
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), box);
    
    textbox = gtk_entry_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(textbox), "commandbox"); // commandbox widget class
    gtk_entry_set_width_chars(GTK_ENTRY(textbox), 30); // Set the width to fit 30 characters
    gtk_box_pack_end(GTK_BOX(box), textbox, FALSE, TRUE, 0);

    // Callback function for Enter detection within textbox
    display = gtk_label_new(NULL);
    gtk_box_pack_start(GTK_BOX(box), display, FALSE, FALSE, 0);
    
    g_signal_connect(
        textbox,
        "activate",
        G_CALLBACK(on_entry_activate),
        display
    );

    // Box Container for close-button in headerbar
    GtkWidget *headerbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(headerbar), headerbox);

    // Exit Session Button
    button = gtk_button_new_with_label("");
    gtk_widget_set_name(button, "close-button");
    gtk_style_context_add_class(gtk_widget_get_style_context(textbox), "close-button"); // close-button widget class

    gtk_box_pack_start(GTK_BOX(headerbox), button, TRUE, TRUE, 0);

    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), window);

    // Connect the "destroy" signal to the window close event
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_closed), NULL);

    // Setting default window size (Note: this line must be kept below the main code to keep it working)
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 300);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

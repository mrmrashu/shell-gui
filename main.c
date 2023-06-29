#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "shell.h"
#include "source.h"
#include "parser.h"
#include "executor.h"
#include "output.h"
char *output_o = "";
int parse_and_execute(struct source_s *src)
{
    skip_white_spaces(src);
    struct token_s *tok = tokenize(src);
    if(tok == &eof_token)
    {
        return 0;
    }
    while(tok && tok != &eof_token)
    {
        struct node_s *cmd = parse_simple_command(tok);
        if(!cmd)
        {
            break;
        }
        int status = do_simple_command(cmd,output_o);

        free_node_tree(cmd);
        tok = tokenize(src);
    }
    return 1;
}

void read_cmd_gui(char *input){


    const size_t len_input = strlen(input)+1;
    char* cmd = malloc(len_input); // A variable to store commands
        // print_prompt1();

        // cmd = read_cmd();

        // For GUI
        strncpy(cmd,input,len_input);

        // If there's an error reading the command, we exit the shell
        if(!cmd){
            exit(EXIT_SUCCESS);
        }

        // If the command is empty (i.e. the user pressed ENTER without writing anything, we skip this input and continue with the loop.
        if (cmd[0]=='\0' || strcmp(cmd,"\n")==0)
        {
            free(cmd);
            return;
        }

        //  If the command is exit, we exit the shell.
        if(strcmp(cmd, "exit\n") == 0)
        {
            free(cmd);
            exit(EXIT_SUCCESS);
            
        }

        //Otherwise, we echo back the command, free the memory we used to store the command, and continue with the loop. 
        // printf("%s\n",cmd);
        struct source_s src;
        src.buffer   = cmd;
        src.bufsize  = strlen(cmd);
        src.curpos   = INIT_SRC_POS;
        parse_and_execute(&src);
    
        free(cmd);
    // exit(EXIT_SUCCESS);
}

void on_window_closed(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

void on_entry_activate(GtkEntry *entry, gpointer data) {
    
    GtkLabel *label = GTK_LABEL(data);

    const gchar *text = gtk_entry_get_text(entry); // Text input in textbox

    char *input = (char *)text;

    read_cmd_gui(input);

    printf("OUT _ %s",output_o);
    // const gchar *current_text = (gchar *)output;
    const gchar *current_text = gtk_label_get_text(label); // Label's Current Text

    // Concatenate the current command with previous for output on display
    gchar *new_text = g_strdup_printf("%s\n%s",current_text, text);
    
    printf("Input Text : %s\n", input);


    gtk_label_set_text(label, new_text);

    g_free(new_text);

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
    GtkWidget *scrollWindow;

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
        window -> headerbar -> headerbox -> box.end -> close-button
        window -> box -> box.end -> entry
    */

    // Parent Container Box window
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), box);
    
    textbox = gtk_entry_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(textbox), "commandbox"); // commandbox widget class
    gtk_entry_set_width_chars(GTK_ENTRY(textbox), 30); // Set the width to fit 30 characters
    gtk_box_pack_end(GTK_BOX(box), textbox, FALSE, TRUE, 0);

    // A Scrolled Window for when the text grows in the Display
    scrollWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_name(scrollWindow, "scrollwindow");
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    // Making the Diaply Element show scrollbar when it grows larger than 400px
    gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scrollWindow), 400);
    gtk_box_pack_start(GTK_BOX(box), scrollWindow, TRUE, TRUE, 0);

    
    display = gtk_label_new("Shell from Project Semicolon ~ #Experimental");
    gtk_widget_set_name(display, "label");

    // Nesting label to scrolled window
    gtk_container_add(GTK_CONTAINER(scrollWindow), display);
    
    g_signal_connect(
        textbox,
        "activate",
        G_CALLBACK(on_entry_activate),
        display
    );

    gtk_label_set_xalign(GTK_LABEL(display), 0.0);
    gtk_label_set_yalign(GTK_LABEL(display), 0.0);

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
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 300);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}



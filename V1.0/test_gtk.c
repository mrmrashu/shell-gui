// To run in Debugging Mode, run via : GTK_DEBUG=interactive ./test_gtk
// Last edited by Ashu Sharma

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

static GtkWidget *text_view;
static GtkWidget *command_entry;
static GtkWidget *stdin_entry; 
static GtkWidget *label;
static GtkWidget *scrolled_window;
static GtkTextBuffer *text_buffer;
static GtkTextTag *tag_stdout;
static GtkTextTag *tag_stderr;
static GtkWidget *username_label;

const char * stdin_input = "";

// Function to update the label
static void update_label(const char *text) {
    gtk_label_set_text(GTK_LABEL(label), text);
}



static void update_directory(const char *directory) {
    if (chdir(directory) != 0) {
        perror("chdir");
        // Handle error: display an error message, etc.
    } else {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            // Now call the update_label function to update the label
            update_label(cwd);
        } else {
            perror("getcwd");
            // Handle error: display an error message, etc.
        }
    }
}

// ...


// ...

static void append_text(const char *text, GtkTextTag *tag) {
    // Get the end iterator of the buffer
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(text_buffer, &iter);

    // Append the text to the buffer with the specified tag
    if (tag) {
        gtk_text_buffer_insert_with_tags(text_buffer, &iter, text, -1, tag, NULL);
    } else {
        gtk_text_buffer_insert(text_buffer, &iter, text, -1);
        gtk_text_buffer_insert(text_buffer, &iter, "\n", -1);  // Add a line break
    }

    // Scroll to the bottom of the text view
    GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
    gtk_adjustment_set_value(adjustment, gtk_adjustment_get_upper(adjustment) - gtk_adjustment_get_page_size(adjustment));
}

// ...



void on_button_clicked(GtkButton *button, gpointer data)
{
    GtkWidget *window = GTK_WIDGET(data);
    gtk_window_close(GTK_WINDOW(window));
}

    

static void run_interactive_command(const char *command, const char *stdin_text) {
    int stdin_pipe[2];
    int stdout_pipe[2];
    int stderr_pipe[2];

    if (pipe(stdin_pipe) == -1 || pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) {
        perror("pipe");
        return;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Child process
        // Redirect stdin, stdout, and stderr
        dup2(stdin_pipe[0], 0);
        dup2(stdout_pipe[1], 1);
        dup2(stderr_pipe[1], 2);

        // Close unused pipe ends
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);

        // Execute the command
        execl("/bin/sh", "sh", "-c", command, NULL);

        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        // Write stdin_text to the child process
        dprintf(stdin_pipe[1], "%s\n", stdin_text);
        close(stdin_pipe[1]);

        char buffer[128];
        ssize_t bytesRead;

        // Read output from the child process
        while ((bytesRead = read(stdout_pipe[0], buffer, sizeof(buffer))) > 0) {
            buffer[bytesRead] = '\0';
            append_text(buffer, tag_stdout);
        }

        // Read errors from the child process
        while ((bytesRead = read(stderr_pipe[0], buffer, sizeof(buffer))) > 0) {
            buffer[bytesRead] = '\0';
            append_text(buffer, tag_stderr);
        }

        close(stdout_pipe[0]);
        close(stderr_pipe[0]);

        // Wait for the child process to finish
        waitpid(pid, NULL, 0);
    }
}

static void on_run_command_button_clicked(GtkWidget *widget, gpointer data) {
    const char *command = gtk_entry_get_text(GTK_ENTRY(command_entry));
    const char *stdin_text = gtk_entry_get_text(GTK_ENTRY(stdin_entry));

    // Display the entered command in the scrolled window with black foreground color
    append_text(command, NULL);

    if (command[0] == 'c' && command[1] == 'd' && command[2] == ' ') {
        update_directory(&command[3]);
    } else {
        run_interactive_command(command, stdin_text);
    }

    gtk_entry_set_text(GTK_ENTRY(command_entry), "");  // Clear the command entry
    gtk_entry_set_text(GTK_ENTRY(stdin_entry), "");    // Clear the stdin entry
}
    
// static void on_stdin_command_button_clicked(GtkWidget *widget, gpointer data) {
//     stdin_input = gtk_entry_get_text(GTK_ENTRY(Std_input));
//     printf("IN = %s",stdin_input);
//     //run_interactive_command(stdin);
//     gtk_entry_set_text(GTK_ENTRY(Std_input), "");  // Clear the command entry
// }


static void on_command_entry_activate(GtkEntry *entry, gpointer user_data) {
    // Called when Enter key is pressed in the command entry
    on_run_command_button_clicked(NULL, NULL);
}


int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Get the username
    const char *username = getlogin();
    
    // Check if the user is root
    if (geteuid() == 0) {
        // User is root
        username_label = gtk_label_new(g_strdup_printf("@%s (root)", username));
        gtk_widget_set_name(username_label, "root-username-label");
    } else {
        // User is not root
        username_label = gtk_label_new(g_strdup_printf("@%s", username));
        gtk_widget_set_name(username_label, "username-label");
    }

    // Set the horizontal alignment of the username_label to end (right-align)
   // gtk_widget_set_halign(username_label, GTK_ALIGN_END);

    // Defining the Main Window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // Window Title for fallback support
    gtk_window_set_title(GTK_WINDOW(window), "Shell 0.2 ~ Project Semicolon");
    
    // Headerbar for custom window style
    GtkWidget *headerbar;
    headerbar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(headerbar), "Shell 0.1 ~ Project Semicolon");

    // Setting the custom header bar as the title bar of the window
    gtk_window_set_titlebar(GTK_WINDOW(window), headerbar);

    // Loading Stylesheets
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path
        (
            provider,
            "./style.css", // Path to CSS file
            NULL
        );

    // Style Context for overriding default styles
    GtkStyleContext *context = gtk_widget_get_style_context(headerbar);
    gtk_style_context_add_provider(
        context,
        GTK_STYLE_PROVIDER(provider), 
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );


    // Redirecting Classes and Styles from Default to Application specific
    // Note : Crucial for working of CSS
    const gchar *class_name = "entry-style";
    GtkStyleContext *context_entry = gtk_widget_get_style_context(window);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_class(context_entry, class_name);

    
    // Text View Widget for displaying output
    
    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_NONE);  // Set wrap mode
    gtk_widget_set_name(text_view, "label");

    // // Create a horizontal scrollbar for the text view
    // GtkWidget *h_scrollbar = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, gtk_scrollable_get_hadjustment(GTK_SCROLLABLE(text_view)));
    // gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    // gtk_container_add(GTK_CONTAINER(scrolled_window), h_scrollbar);

    // Wrap the text view in a scrolled window
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    // Intial Current Working Directory
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    label = gtk_label_new(cwd);
    
    // Nesting Text View inside Scrolled Window
    gtk_container_add
        (GTK_CONTAINER
            (scrolled_window),
            text_view
        );


    stdin_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(stdin_entry), "Enter stdin...");
    gtk_entry_set_width_chars(GTK_ENTRY(stdin_entry), 30); // Set the width to fit 30 characters

    // Input Textbox for Commands
    command_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text
        (GTK_ENTRY(command_entry),
            "Enter command..."
        );

    gtk_style_context_add_class(gtk_widget_get_style_context(command_entry), "commandbox"); // commandbox widget class
    gtk_style_context_add_class(gtk_widget_get_style_context(stdin_entry), "Stdinputbox"); // commandbox widget class

    gtk_entry_set_width_chars(GTK_ENTRY(command_entry), 30); // Set the width to fit 30 characters

    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));


    // Create tags for different text styles
    tag_stdout = gtk_text_buffer_create_tag(text_buffer, "stdout", "foreground", "blue", "background", "#ADD8E6", "left_margin", 10, NULL);
    tag_stderr = gtk_text_buffer_create_tag(text_buffer, "stderr", "foreground", "red", "background", "#FFCCCB", "left_margin", 10, NULL);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Connect the activate signal to the callback for Enter key presses
    g_signal_connect(command_entry, "activate", G_CALLBACK(on_command_entry_activate), NULL);
    //g_signal_connect(Std_input, "activate", G_CALLBACK(on_stdin_command_button_clicked), NULL);

    

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    // Add the original label to the labels box
    gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);

     // Create a new horizontal box for labels
    GtkWidget *labels_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    // Add the original label to the labels box
    gtk_box_pack_start(GTK_BOX(labels_box), label, FALSE, FALSE, 0);

    // Set the horizontal alignment of the username_label to end (right-align)
    gtk_widget_set_halign(username_label, GTK_ALIGN_END);

    // Add the username label to the labels box
    gtk_box_pack_end(GTK_BOX(labels_box), username_label, FALSE, FALSE, 0);

    // Add the labels box to the main vertical box
    gtk_box_pack_start(GTK_BOX(box), labels_box, FALSE, FALSE, 0);
    // Add the command entry to the box
    //gtk_box_pack_start(GTK_BOX(box), username_label, FALSE, FALSE, 0);
    //gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), stdin_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), command_entry, FALSE, FALSE, 0);
    // Add the scrolled window to the box, expanding to fill available space

    // Add the box to the window
    gtk_container_add(GTK_CONTAINER(window), box);

    // Box Container for close-button in headerbar
    GtkWidget *headerbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(headerbar), headerbox);

    // Exit Session Button
    GtkWidget *button;
    button = gtk_button_new();
    gtk_widget_set_name(button, "close-button");
    gtk_style_context_add_class(gtk_widget_get_style_context(button), "close-button"); // close-button widget class

    gtk_box_pack_start(GTK_BOX(headerbox), button, TRUE, TRUE, 0);

    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), window);

    // Connect the "destroy" signal to the window close event
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_window_close), NULL);

    // Setting default window size (Note: this line must be kept below the main code to keep it working)
    gtk_window_set_default_size(GTK_WINDOW(window), 750, 450);

   
    // Show all the widgets
    gtk_widget_show_all(window);


    // Start the GTK main loop
    gtk_main();
    // g_free(gtk_label_get_text(GTK_LABEL(username_label)));
    // g_free(gtk_label_get_text(GTK_LABEL(label)));

    return 0;
}
// To run in Debugging Mode, run via : GTK_DEBUG=interactive ./test_gtk
// Last edited by Ashu Sharma

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

static GtkWidget *text_view;
static GtkWidget *command_entry;
static GtkWidget *Std_input;
static GtkTextBuffer *text_buffer;
static GtkTextTag *tag_stdout;
static GtkTextTag *tag_stderr;
const char * stdin_input = "";

static GtkWidget *label;

static void update_directory(const char *directory) {
    //const char *directory = gtk_entry_get_text(entry);

    if (chdir(directory) != 0) {
        perror("chdir");
        // Handle error: display an error message, etc.
    } else {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            gtk_label_set_text(GTK_LABEL(label), cwd);
        } else {
            perror("getcwd");
            // Handle error: display an error message, etc.
        }
    }
}

static void append_text(const char *text, GtkTextTag *tag) {
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(text_buffer, &iter);

    // Append the text with a newline character
    gchar *text_with_newline = g_strdup_printf("%s\n", text);
    gtk_text_buffer_insert_with_tags(text_buffer, &iter, text_with_newline, -1, tag, NULL);
    g_free(text_with_newline);
}

void on_button_clicked(GtkButton *button, gpointer data)
{
    GtkWidget *window = GTK_WIDGET(data);
    gtk_window_close(GTK_WINDOW(window));
}

static void run_interactive_command(const char *command) {
    FILE *fp;

    // Create a temporary file for stdin
    char temp_filename[] = "/tmp/stdin_tempfile_XXXXXX";
    int stdin_temp_fd = mkstemp(temp_filename);

    if (stdin_temp_fd == -1) {
        perror("mkstemp");
        return;
    }

    // Write stdin_input to the temporary file
    write(stdin_temp_fd, stdin_input, strlen(stdin_input)+1);
    close(stdin_temp_fd);

    // Create pipes for stdout and stderr
    int stdout_pipe[2];
    int stderr_pipe[2];
    if (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) {
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
        // Redirect stdout and stderr
        dup2(stdout_pipe[1], 1);
        dup2(stderr_pipe[1], 2);

        // Close unused pipe ends
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);

        // Execute the command with stdin redirected from the temporary file
        execl("/bin/sh", "sh", "-c", command, "<", temp_filename, NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        // Read the output and errors from the child process and append them to the text view
        char buffer[128];
        ssize_t bytesRead;

        while ((bytesRead = read(stdout_pipe[0], buffer, sizeof(buffer))) > 0) {
            buffer[bytesRead] = '\0';
            append_text(buffer, tag_stdout);
        }

        while ((bytesRead = read(stderr_pipe[0], buffer, sizeof(buffer))) > 0) {
            buffer[bytesRead] = '\0';
            // Append errors to the text view with the error tag
            append_text(buffer, tag_stderr);
        }

        close(stdout_pipe[0]);
        close(stderr_pipe[0]);

        // Wait for the child process to finish
        waitpid(pid, NULL, 0);

        // Clean up the temporary file
        unlink(temp_filename);
    }
}


static void on_run_command_button_clicked(GtkWidget *widget, gpointer data) {
    const char *command = gtk_entry_get_text(GTK_ENTRY(command_entry));
    if (command[0] == 'c' && command[1] == 'd' && command[2] == ' ')
    {
        update_directory(&command[3]);
    }
    else
    {
        run_interactive_command(command);
    }
    gtk_entry_set_text(GTK_ENTRY(command_entry), "");  // Clear the command entry
}
static void on_stdin_command_button_clicked(GtkWidget *widget, gpointer data) {
    stdin_input = gtk_entry_get_text(GTK_ENTRY(Std_input));
    printf("IN = %s",stdin_input);
    //run_interactive_command(stdin);
    gtk_entry_set_text(GTK_ENTRY(Std_input), "");  // Clear the command entry
}


static void on_command_entry_activate(GtkEntry *entry, gpointer user_data) {
    // Called when Enter key is pressed in the command entry
    on_run_command_button_clicked(NULL, NULL);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

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
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
    gtk_widget_set_name(text_view, "label");

    // Wrap the text view in a scrolled window
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    label = gtk_label_new(" ");
    
    // Nesting Text View inside Scrolled Window
    gtk_container_add
        (GTK_CONTAINER
            (scrolled_window),
            text_view
        );


    Std_input = gtk_entry_new();
    gtk_entry_set_placeholder_text
        (GTK_ENTRY(Std_input),
            "Enter Input..."
        );
    // Input Textbox for Commands
    command_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text
        (GTK_ENTRY(command_entry),
            "Enter command..."
        );

    gtk_style_context_add_class(gtk_widget_get_style_context(command_entry), "commandbox"); // commandbox widget class
    gtk_entry_set_width_chars(GTK_ENTRY(command_entry), 30); // Set the width to fit 30 characters

    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));


    // Create tags for different text styles
    tag_stdout = gtk_text_buffer_create_tag(text_buffer, "stdout", "foreground", "blue", "left_margin", 10, NULL);
    tag_stderr = gtk_text_buffer_create_tag(text_buffer, "stderr", "foreground", "red", "left_margin", 10, NULL);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Connect the activate signal to the callback for Enter key presses
    g_signal_connect(command_entry, "activate", G_CALLBACK(on_command_entry_activate), NULL);
    g_signal_connect(Std_input, "activate", G_CALLBACK(on_stdin_command_button_clicked), NULL);


    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    // Add the command entry to the box
    gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), Std_input, FALSE, FALSE, 0);
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
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 300);

   
    // Show all the widgets
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();

    return 0;
}
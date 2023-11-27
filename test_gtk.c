#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

static GtkWidget *text_view;
static GtkWidget *command_entry;
static GtkTextBuffer *text_buffer;
static GtkTextTag *tag_stdout;
static GtkTextTag *tag_stderr;

static void append_text(const char *text, GtkTextTag *tag) {
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(text_buffer, &iter);
    gtk_text_buffer_insert_with_tags(text_buffer, &iter, text, -1, tag, NULL);
}

static void run_interactive_command(const char *command) {
    FILE *fp;

    // Create pipes for stdin, stdout, and stderr
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
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);

        // Redirect stdin, stdout, and stderr
        dup2(stdin_pipe[0], 0);
        dup2(stdout_pipe[1], 1);
        dup2(stderr_pipe[1], 2);

        // Close unused pipe ends
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
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

        // Write the command to the child process
        write(stdin_pipe[1], command, strlen(command));
        close(stdin_pipe[1]);

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
        waitpid(pid, NULL, 0);  // Wait for the child process to finish
    }
}

static void on_run_command_button_clicked(GtkWidget *widget, gpointer data) {
    const char *command = gtk_entry_get_text(GTK_ENTRY(command_entry));
    run_interactive_command(command);
    gtk_entry_set_text(GTK_ENTRY(command_entry), "");  // Clear the command entry
}

static void on_command_entry_activate(GtkEntry *entry, gpointer user_data) {
    // Called when Enter key is pressed in the command entry
    on_run_command_button_clicked(NULL, NULL);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *run_command_button = gtk_button_new_with_label("Run Command");

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);

    // Wrap the text view in a scrolled window
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    command_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(command_entry), "Enter command...");

    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    // Create tags for different text styles
    tag_stdout = gtk_text_buffer_create_tag(text_buffer, "stdout", "foreground", "blue", "left_margin", 10, NULL);
    tag_stderr = gtk_text_buffer_create_tag(text_buffer, "stderr", "foreground", "red", "left_margin", 10, NULL);

    g_signal_connect(run_command_button, "clicked", G_CALLBACK(on_run_command_button_clicked), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Connect the activate signal to the callback for Enter key presses
    g_signal_connect(command_entry, "activate", G_CALLBACK(on_command_entry_activate), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    // Header bar with a title
    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Interactive Shell");
    gtk_box_pack_start(GTK_BOX(box), header_bar, FALSE, FALSE, 0);

    // Add the command entry and run button to the box
    gtk_box_pack_start(GTK_BOX(box), command_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), run_command_button, FALSE, FALSE, 0);

    // Add the scrolled window to the box, expanding to fill available space
    gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);

    // Add the box to the window
    gtk_container_add(GTK_CONTAINER(window), box);

    // Set the size of the window
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    // Show all the widgets
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();

    return 0;
}

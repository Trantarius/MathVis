#include <gtk/gtk.h>
#include "ui.hpp"

void MainUI::init(){
	window = (GtkApplicationWindow*) gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "MathVis");
	gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);

	paned = (GtkPaned*) gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_window_set_child(GTK_WINDOW(window),(GtkWidget*)paned);
}

static void activate (GtkApplication* app, gpointer user_data){
	main_ui.init();
	gtk_window_present (GTK_WINDOW (main_ui.window));
}

int main (int argc, char **argv){
	int status;

	main_ui.app = gtk_application_new ("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect (main_ui.app, "activate", G_CALLBACK (activate), NULL);
	status = g_application_run (G_APPLICATION (main_ui.app), argc, argv);
	g_object_unref (main_ui.app);

	return status;
}

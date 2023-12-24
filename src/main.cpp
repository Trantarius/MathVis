#include <gtk/gtk.h>
#include "ui.hpp"

void MainUI::init(){
	window = GTK_APPLICATION_WINDOW( gtk_application_window_new (app));
	gtk_window_set_title (GTK_WINDOW (window), "MathVis");
	gtk_window_set_default_size (GTK_WINDOW (window), 1200, 800);

	paned = GTK_PANED( gtk_paned_new(GTK_ORIENTATION_HORIZONTAL));
	gtk_window_set_child(GTK_WINDOW(window),GTK_WIDGET(paned));

	defs_panel.init();
	gtk_paned_set_start_child(paned,defs_panel);
	widget_set_margin(defs_panel,4);

	output_panel.init();
	gtk_paned_set_end_child(paned,output_panel);
	widget_set_margin(output_panel,4);

	color_dialog = gtk_color_dialog_new();
}

void OutputPanel::init(){
	tab_pane=GTK_NOTEBOOK(gtk_notebook_new());
	graph_area=GTK_DRAWING_AREA(gtk_drawing_area_new());
	gtk_notebook_append_page(tab_pane,GTK_WIDGET(graph_area),NULL);
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

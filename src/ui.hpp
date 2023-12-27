#pragma once
#include <gtk/gtk.h>
#include <vector>

class DefsEntry{
	int idx;
public:

	GtkFrame* frame=nullptr;
	GtkBox* hbox=nullptr;
	GtkLabel* linenum=nullptr;
	struct{
		GtkBox* hbox=nullptr;
		GtkSeparator* left=nullptr;
		GtkSeparator* right=nullptr;
		GtkBox* vbox=nullptr;
		GtkTextView* text_view=nullptr;
		GtkLabel* error=nullptr;
	} textedit;
	struct{
		GtkBox* vbox=nullptr;
		GtkButton* remove_button=nullptr;
		GtkCheckButton* display_toggle=nullptr;
		GtkButton* menu_button=nullptr;
		GtkColorDialogButton* color_button=nullptr;
	} options;

	void init();
	~DefsEntry();

	void set_idx(int);
	int get_idx() const{ return idx; }

	operator GtkWidget*() const {return GTK_WIDGET(frame);}
};

struct DefsPanel{
	GtkFrame* frame=nullptr;
	GtkBox* vbox=nullptr;

	struct{
		GtkBox* hbox=nullptr;
		GtkButton* add_button=nullptr;
	} header;

	GtkSeparator* separator=nullptr;

	struct{
		GtkScrolledWindow* scroller=nullptr;
		GtkBox* vbox=nullptr;

		std::vector<DefsEntry*> entries;
	} defs;

	void init();

	static void _on_add_button_press(GtkWidget*,gpointer);
	void add_entry(DefsEntry*);
	void remove_entry(DefsEntry*);

	operator GtkWidget*() const {return GTK_WIDGET(frame);}
};

struct OutputPanel{
	GtkNotebook* tab_pane=nullptr;
	GtkDrawingArea* graph_area=nullptr;

	void init();

	operator GtkWidget*() const {return GTK_WIDGET(tab_pane);}
};

struct MainUI{
	GtkApplication* app=nullptr;
	GtkApplicationWindow* window=nullptr;
	GtkPaned* paned=nullptr;

	GtkColorDialog* color_dialog=nullptr;

	DefsPanel defs_panel;
	OutputPanel output_panel;

	void init();
};

inline MainUI main_ui;


//convenience

inline void widget_set_margin(GtkWidget* widget, int margin){
	gtk_widget_set_margin_start(widget,margin);
	gtk_widget_set_margin_end(widget,margin);
	gtk_widget_set_margin_top(widget,margin);
	gtk_widget_set_margin_bottom(widget,margin);
}

inline void widget_set_margin(GtkWidget* widget, int horz, int vert){
	gtk_widget_set_margin_start(widget,horz);
	gtk_widget_set_margin_end(widget,horz);
	gtk_widget_set_margin_top(widget,vert);
	gtk_widget_set_margin_bottom(widget,vert);
}

inline void widget_set_margin(GtkWidget* widget, int margin_start, int margin_end, int margin_top, int margin_bottom){
	gtk_widget_set_margin_start(widget,margin_start);
	gtk_widget_set_margin_end(widget,margin_end);
	gtk_widget_set_margin_top(widget,margin_top);
	gtk_widget_set_margin_bottom(widget,margin_bottom);
}

inline void widget_add_margin(GtkWidget* widget, int margin){
	gtk_widget_set_margin_start(widget,gtk_widget_get_margin_start(widget)+margin);
	gtk_widget_set_margin_end(widget,gtk_widget_get_margin_end(widget)+margin);
	gtk_widget_set_margin_top(widget,gtk_widget_get_margin_top(widget)+margin);
	gtk_widget_set_margin_bottom(widget,gtk_widget_get_margin_bottom(widget)+margin);
}

inline void widget_add_margin(GtkWidget* widget, int horz, int vert){
	gtk_widget_set_margin_start(widget,gtk_widget_get_margin_start(widget)+horz);
	gtk_widget_set_margin_end(widget,gtk_widget_get_margin_end(widget)+horz);
	gtk_widget_set_margin_top(widget,gtk_widget_get_margin_top(widget)+vert);
	gtk_widget_set_margin_bottom(widget,gtk_widget_get_margin_bottom(widget)+vert);
}

inline void widget_add_margin(GtkWidget* widget, int margin_start, int margin_end, int margin_top, int margin_bottom){
	gtk_widget_set_margin_start(widget,gtk_widget_get_margin_start(widget)+margin_start);
	gtk_widget_set_margin_end(widget,gtk_widget_get_margin_end(widget)+margin_end);
	gtk_widget_set_margin_top(widget,gtk_widget_get_margin_top(widget)+margin_top);
	gtk_widget_set_margin_bottom(widget,gtk_widget_get_margin_bottom(widget)+margin_bottom);
}

inline void widget_set_align(GtkWidget* widget, GtkAlign horz, GtkAlign vert){
	gtk_widget_set_halign(widget,horz);
	gtk_widget_set_valign(widget,vert);
}

inline void widget_set_align(GtkWidget* widget, GtkAlign align){
	gtk_widget_set_halign(widget,align);
	gtk_widget_set_valign(widget,align);
}

inline void widget_set_expand(GtkWidget* widget, bool horz, bool vert){
	gtk_widget_set_hexpand(widget,horz);
	gtk_widget_set_vexpand(widget,vert);
}

inline void widget_set_expand(GtkWidget* widget, bool expand){
	gtk_widget_set_hexpand(widget,expand);
	gtk_widget_set_vexpand(widget,expand);
}

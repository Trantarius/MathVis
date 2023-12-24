#include <gtk/gtk.h>
#pragma once



struct FuncsPanel{


	void init();
};

struct OutputPanel{


	void init();
};

struct MainUI{
	GtkApplication* app=nullptr;
	GtkApplicationWindow* window=nullptr;
	GtkPaned* paned=nullptr;

	FuncsPanel funcs_panel;
	OutputPanel output_panel;

	void init();
};

inline MainUI main_ui;

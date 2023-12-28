#include "parser.hpp"
#include "ui.hpp"
#include <string>

void DefsPanel::init() {
	frame = GTK_FRAME(gtk_frame_new(NULL));
	gtk_widget_set_size_request(GTK_WIDGET(frame),0,0);
	vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
	gtk_frame_set_child(frame, GTK_WIDGET(vbox));

	header.hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
	header.add_button =
	GTK_BUTTON(gtk_button_new_from_icon_name("list-add-symbolic"));
	widget_set_margin(GTK_WIDGET(header.add_button), 4);
	g_signal_connect(header.add_button, "clicked",
									 G_CALLBACK(DefsPanel::_on_add_button_press), NULL);
	gtk_box_append(header.hbox, GTK_WIDGET(header.add_button));
	gtk_box_append(vbox, GTK_WIDGET(header.hbox));

	separator = GTK_SEPARATOR(gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
	gtk_box_append(vbox, GTK_WIDGET(separator));

	defs.scroller = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new());
	gtk_scrolled_window_set_policy(defs.scroller, GTK_POLICY_NEVER,
																 GTK_POLICY_AUTOMATIC);
	gtk_widget_set_vexpand(GTK_WIDGET(defs.scroller), true);
	defs.vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 4));
	widget_set_margin(GTK_WIDGET(defs.vbox), 4);
	gtk_scrolled_window_set_child(defs.scroller, GTK_WIDGET(defs.vbox));
	gtk_box_append(vbox, GTK_WIDGET(defs.scroller));
}

void DefsPanel::_on_add_button_press(GtkWidget *, gpointer) {
	DefsEntry *entry = new DefsEntry();
	entry->init();
	main_ui.defs_panel.add_entry(entry);
}

void DefsPanel::add_entry(DefsEntry *entry) {
	gtk_box_append(defs.vbox, *entry);
	defs.entries.push_back(entry);
	entry->set_idx(defs.entries.size() - 1);
}

void DefsPanel::remove_entry(DefsEntry *entry) {
	gtk_box_remove(defs.vbox, *entry);
	typedef decltype(defs.entries)::iterator Iter;
	for (Iter it = defs.entries.begin(); it != defs.entries.end(); it++) {
		if (*it == entry) {
			defs.entries.erase(it);
			break;
		}
	}
	for (int n = 0; n < defs.entries.size(); n++) {
		defs.entries[n]->set_idx(n);
	}
	delete entry;
}

void DefsEntry::init() {
	frame = GTK_FRAME(gtk_frame_new(NULL));
	g_object_ref_sink(frame);
	gtk_widget_set_size_request(GTK_WIDGET(frame),0,-1);
	hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
	gtk_frame_set_child(frame, GTK_WIDGET(hbox));

	linenum = GTK_LABEL(gtk_label_new("#"));
	widget_set_margin(GTK_WIDGET(linenum), 4);
	gtk_box_append(hbox, GTK_WIDGET(linenum));

	// textedit
	textedit.hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
	widget_set_expand(GTK_WIDGET(textedit.hbox), true, false);

	textedit.left = GTK_SEPARATOR(gtk_separator_new(GTK_ORIENTATION_VERTICAL));
	gtk_box_append(textedit.hbox, GTK_WIDGET(textedit.left));

	textedit.vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
	gtk_box_append(textedit.hbox, GTK_WIDGET(textedit.vbox));

	textedit.text_view = GTK_TEXT_VIEW(gtk_text_view_new());
	widget_set_expand(GTK_WIDGET(textedit.text_view), true, true);
	widget_set_align(GTK_WIDGET(textedit.text_view), GTK_ALIGN_FILL);
	gtk_box_append(textedit.vbox, GTK_WIDGET(textedit.text_view));

	textedit.error = GTK_LABEL(gtk_label_new("Error Message"));
	widget_set_margin(GTK_WIDGET(textedit.error), 4);
	widget_set_align(GTK_WIDGET(textedit.error), GTK_ALIGN_START, GTK_ALIGN_CENTER);
	gtk_widget_set_overflow(GTK_WIDGET(textedit.error),GTK_OVERFLOW_HIDDEN);
	gtk_box_append(textedit.vbox, GTK_WIDGET(textedit.error));

	textedit.right = GTK_SEPARATOR(gtk_separator_new(GTK_ORIENTATION_VERTICAL));
	gtk_box_append(textedit.hbox, GTK_WIDGET(textedit.right));

	gtk_box_append(hbox, GTK_WIDGET(textedit.hbox));

	// options
	options.vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 4));
	widget_set_margin(GTK_WIDGET(options.vbox), 4);

	options.remove_button =
	GTK_BUTTON(gtk_button_new_from_icon_name("edit-delete-symbolic"));
	widget_set_align(GTK_WIDGET(options.remove_button), GTK_ALIGN_CENTER);
	gtk_box_append(options.vbox, GTK_WIDGET(options.remove_button));

	options.display_toggle = GTK_CHECK_BUTTON(gtk_check_button_new());
	gtk_check_button_set_active(options.display_toggle, true);
	widget_set_align(GTK_WIDGET(options.display_toggle), GTK_ALIGN_CENTER);
	gtk_box_append(options.vbox, GTK_WIDGET(options.display_toggle));

	options.menu_button =
	GTK_BUTTON(gtk_button_new_from_icon_name("open-menu-symbolic"));
	widget_set_align(GTK_WIDGET(options.menu_button), GTK_ALIGN_CENTER);
	gtk_box_append(options.vbox, GTK_WIDGET(options.menu_button));

	options.color_button = GTK_COLOR_DIALOG_BUTTON(
		gtk_color_dialog_button_new(main_ui.color_dialog));
	widget_set_align(GTK_WIDGET(options.color_button), GTK_ALIGN_CENTER);
	gtk_box_append(options.vbox, GTK_WIDGET(options.color_button));

	gtk_box_append(hbox, GTK_WIDGET(options.vbox));

	g_signal_connect(gtk_text_view_get_buffer(textedit.text_view), "changed",
									 G_CALLBACK(DefsEntry::_on_text_changed), this);
}

DefsEntry::~DefsEntry() {
	if (frame) {
		g_object_unref(frame);
	}
}

void DefsEntry::set_idx(int to) {
	idx = to;
	if (linenum) {
		std::string linenum_text = std::to_string(to);
		gtk_label_set_label(linenum, linenum_text.c_str());
	}
}

void DefsEntry::_on_text_changed(GtkTextBuffer *buffer, gpointer userdata) {
	DefsEntry *entry = (DefsEntry *)userdata;
	GtkTextIter start, end;
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);

	const char *text = gtk_text_buffer_get_text(buffer, &start, &end, false);
	Expr ex;
	string message="what";
	try{
		ex = parse(text);
		try{
			ex = ex.evaluate();
			message=ex.to_string(true);
		}catch(ExprError err){
			message=err.what;
		}
	}catch(ParseFail pf){
		message=pf.reason;
	}

	gtk_label_set_label(entry->textedit.error,message.c_str());
	delete [] text;
}

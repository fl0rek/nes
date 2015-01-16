#include "marshall.h"
#include "backend.h"
#include "avl.h"

#include <stdint.h>
#include <panel.h>
#include <stdio.h>
#include <menu.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))



uint8_t sort_by = 0;
MENU* generate_menu(avltree* t) {
	node* n = first(t);
	ITEM** item = calloc(get_node_num(t), sizeof(*item));

	int i;
	for(i = 0; n; i++) {
		record* r = get_data(n);
		const char* name = magic_get(r, r->o_name);
		const char* surname = magic_get(r, r->o_surname);
		const char* workspace = magic_get(r, r->o_workspace);
		const char* computername = magic_get(r, r->o_computername);
		const uint8_t* ip = (uint8_t*) get_ip(r);

		int linesize = 81;
		int freesize = linesize - 3 - 16;
		char* line = calloc(linesize, sizeof(*line));
		snprintf(line, linesize, "%*1$s %*1$s %*1$s %*1$s %s.%s.%s.%s", freesize/4,
				computername, workspace, name, surname, 
				ip[0], ip[1], ip[2], ip[3] );
		item[i] = new_item(line, i);
		n = next(t, n);
	}
	return new_menu(item);
}

char* topmenu[] = {
	"Save",
	"Load",
	"Edit enum type",
	"Quit",
	0
};

char* topmenu_keys[] = {
	"F1",
	"F2",
	"F3",
	"F4",
	0
};

int8_t record_compare_name(void* lhs, void* rhs);

main(int argc, char** argv) {
	//int lines = 10, cols = 40, y = 2, x = 4, i;
	int maxlines;
	int maxcols;

	initscr();
	cbreak();
	noecho();

	getmaxyx(stdscr, maxlines, maxcols);

	WINDOW* win_list = newwin(maxlines-1, maxcols, 1, 0);

	ITEM** item = calloc(ARRAY_SIZE(topmenu)+1, sizeof(ITEM*));
	int it;
	for(it = 0; it < ARRAY_SIZE(topmenu); it++)
		item[it] = new_item(topmenu_keys[it], topmenu[it]);

	MENU* men_top = new_menu(item);
	set_menu_format(men_top, 1, 5);
	post_menu(men_top);


	box(win_list, 0, 0);

	/* Attach a panel to each window */ 	/* Order is bottom up */
	PANEL* pan_list = new_panel(win_list);

	uint8_t ip[] = {10, 10, 10, 10};

	avltree* s_name = avl_init_tree(record_compare_name);
	record* r = create_record(ip, "computername1", "name1", "surname1", 1, "workspace1");
	insert(s_name, r);
	r = create_record(ip, "computername2", "name2", "surname2", 2, "workspace2");
	insert(s_name, r);
	r = create_record(ip, "computername3", "name3", "surname3", 3, "workspace3");
	insert(s_name, r);

	MENU* men_list = generate_menu(s_name);

	/* Update the stacking order. 2nd panel will be on top */
	update_panels();

	/* Show it on the screen */
	doupdate();

	char c;
	while((c = wgetch(win_list)) != KEY_F(4))
	{       
		switch(c) {
		};
		wrefresh(win_list);
	}	

	endwin();	

}

int8_t record_compare_name(void* lhs, void* rhs) {
	record* lhr = (record*) lhs;
	record* rhr = (record*) rhs;
	char* lhname = magic_get(lhs, lhr->o_name);
	char* rhname = magic_get(rhs, rhr->o_name);
	return strcmp(lhname, rhname);
}

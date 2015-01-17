#include "marshall.h"
#include "backend.h"
#include "avl.h"

#include <stdint.h>
#include <panel.h>
#include <stdio.h>
#include <menu.h>

#include <assert.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))



char* header = 0;
uint8_t sort_by = 0;
ITEM** generate_menu(avltree* t, int cols) {
	if(!t)
		return 0;
	node* n = first(t);
	ITEM** item = calloc(get_node_num(t) +1, sizeof(*item));

	int linesize = cols;
	int freesize = linesize -3 -16 -4;


	int i;
	for(i = 0; n; i++) {
		record* r = get_data(n);
		const char* name = magic_get(r, r->o_name);
		const char* surname = magic_get(r, r->o_surname);
		const char* workspace = magic_get(r, r->o_workspace);
		const char* computername = magic_get(r, r->o_computername);
		const uint8_t* ip = (uint8_t*) get_ip(r);

debugme:

		assert(name);
		assert(surname);
		assert(workspace);
		assert(computername);
		assert(ip);

		char* line = calloc(linesize, sizeof(*line));
		char* ord = calloc(4, sizeof(*ord));
		
		snprintf(line, linesize, "%2$*1$s", 10, "abcdefghijklmno");
		snprintf(line, linesize, "%2$*1$s %3$*1$s %4$*1$s %5$*1$s %6$"PRIu8".%7$"PRIu8".%8$"PRIu8".%9$"PRIu8, -freesize/4,
				computername, workspace, name, surname, 
				ip[0], ip[1], ip[2], ip[3]);
				
		snprintf(ord, 4, "%3d", i);
		item[i] = new_item(ord, line);
		n = next(t, n);
	}

	if(header)
		snprintf(header, linesize, "%2$*1$s %3$*1$s %4$*1$s %5$*1$s %6$s", -freesize/4,
			"Computer Name", "Workgroup", "Name", "Surname", "IP");
	return item;
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
int8_t record_compare_computername(void* lhs, void* rhs);
int8_t record_compare_surname(void* lhs, void* rhs);
int8_t record_compare_workspace(void* lhs, void* rhs);

avltree** sorts;
char* sorts_name[] = {
	"computername",
	"workspace"
	"name",
	"surname",
};

uint8_t sort_num = 0;


void free_items(ITEM** i) {
	while(*i) {
		free_item(*i++);
	}
}

ITEM** next_sort(int linesize) {
	sort_num = (sort_num+1) % 4;
	return generate_menu(sorts[sort_num], linesize);
}

ITEM** prev_sort(int linesize) {
	sort_num = sort_num ? sort_num -1 : 3;
	return generate_menu(sorts[sort_num], linesize);
}

void insert_all(record* r) {
	uint8_t i;
	for(i = 0; i < 4; i++) {
		insert(sorts[i], r);
	}
}

main(int argc, char** argv) {
	//int lines = 10, cols = 40, y = 2, x = 4, i;
	int maxlines;
	int maxcols;

	initscr();
	cbreak();
	noecho();

	keypad(stdscr, TRUE);

	getmaxyx(stdscr, maxlines, maxcols);

	header = calloc(maxcols, sizeof(*header));

	WINDOW* win_list = newwin(maxlines-1, maxcols, 1, 0);

	ITEM** item = calloc(ARRAY_SIZE(topmenu)+1, sizeof(ITEM*));
	int it;
	for(it = 0; it < ARRAY_SIZE(topmenu); it++)
		item[it] = new_item(topmenu_keys[it], topmenu[it]);

	MENU* men_top = new_menu(item);
	set_menu_format(men_top, 1, 5);
	post_menu(men_top);

	sorts = calloc(4, sizeof(*sorts));
	sorts[0] = avl_init_tree(record_compare_computername);
	sorts[1] = avl_init_tree(record_compare_workspace);
	sorts[2] = avl_init_tree(record_compare_name);
	sorts[3] = avl_init_tree(record_compare_surname);

	box(win_list, 0, 0);

	/* Attach a panel to each window */ 	/* Order is bottom up */
	PANEL* pan_list = new_panel(win_list);

	uint8_t ip[] = {10, 10, 10, 10};

	//avltree* s_name = avl_init_tree(record_compare_name);
	record* r = create_record(ip, "cn one", "n ooo", "sn abc", 1, "w ddd");
	insert_all(r);
	r = create_record(ip, "cn two", "n a", "sn dda", 2, "w ooo");
	insert_all(r);
	r = create_record(ip, "cn thr", "n 2", "sn oo", 3, "w iii");
	insert_all(r);

	MENU* men_list = new_menu(generate_menu(sorts[0], maxcols-2));

	set_menu_win(men_list, win_list);
	set_menu_sub(men_list, derwin(win_list, maxlines-6, maxcols-2, 3, 1));

	set_menu_mark(men_list, " * ");

	post_menu(men_list);
	wrefresh(win_list);


	/* Update the stacking order. 2nd panel will be on top */
	update_panels();

	/* Show it on the screen */
	doupdate();

	int c;
	ITEM** i;
	while((c = wgetch(stdscr)) != KEY_F(4) && c != 'q') {       
		mvprintw(3, 2, "%s", header);

		switch(c) {
			case KEY_DOWN:
			case 'j':
				menu_driver(men_list, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
			case 'k':
				menu_driver(men_list, REQ_UP_ITEM);
				break;
			case KEY_LEFT:
				i = menu_items(men_list);
				unpost_menu(men_list);
				set_menu_items(men_list, prev_sort(maxcols-2));
				post_menu(men_list);
				free_items(i);
				refresh();
				break;
			case KEY_RIGHT:
				i = menu_items(men_list);
				unpost_menu(men_list);
				set_menu_items(men_list, next_sort(maxcols-2));
				post_menu(men_list);
				free_items(i);
				refresh();
				break;
			case KEY_NPAGE:
				menu_driver(men_list, REQ_SCR_DPAGE);
				break;
			case KEY_PPAGE:
				menu_driver(men_list, REQ_SCR_UPAGE);
				break;

		};
		mvprintw(maxlines - 2, 0, "got some shit %3d [%3d] %2d %15s", c, KEY_F(1), sort_num, sorts_name[sort_num]);
		wrefresh(win_list);
	}	

	unpost_menu(men_list);
	free_menu(men_list);
	endwin();	

}

int8_t record_compare_name(void* lhs, void* rhs) {
	record* lhr = (record*) lhs;
	record* rhr = (record*) rhs;
	const char* lhname = magic_get(lhs, lhr->o_name);
	const char* rhname = magic_get(rhs, rhr->o_name);
	return strcmp(lhname, rhname); 
}
int8_t record_compare_surname(void* lhs, void* rhs) {
	record* lhr = (record*) lhs;
	record* rhr = (record*) rhs;
	const char* lhsurname = magic_get(lhs, lhr->o_surname);
	const char* rhsurname = magic_get(rhs, rhr->o_surname);
	return strcmp(lhsurname, rhsurname); 
}
int8_t record_compare_computername(void* lhs, void* rhs) {
	record* lhr = (record*) lhs;
	record* rhr = (record*) rhs;
	const char* lhcomputername = magic_get(lhs, lhr->o_computername);
	const char* rhcomputername = magic_get(rhs, rhr->o_computername);
	return strcmp(lhcomputername, rhcomputername); 
}
int8_t record_compare_workspace(void* lhs, void* rhs) {
	record* lhr = (record*) lhs;
	record* rhr = (record*) rhs;
	const char* lhworkspace = magic_get(lhs, lhr->o_workspace);
	const char* rhworkspace = magic_get(rhs, rhr->o_workspace);
	return strcmp(lhworkspace, rhworkspace); 
}

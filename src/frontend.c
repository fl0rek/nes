#include "marshall.h"
#include "backend.h"
#include "frontend.h"
#include "avl.h"

#include <stdint.h>
#include <stdio.h>
#include <menu.h>
#include <form.h>

#include <string.h>
#include <assert.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

char* header = 0;
char* footer[200];
uint8_t sort_by = 0;
ITEM** generate_menu(avltree* t, int cols) {
	if(!t)
		return 0;
	node* n = first(t);
	ITEM** item = calloc(get_node_num(t) +1, sizeof(*item));

	int linesize = cols;
	int freesize = linesize -3 -16 -4;

	endwin();

	int i;
	if(!n) {
		char empty[] = "EMPTY";
		char ord[] = "0";
		char* line = calloc(linesize, sizeof(*line));

		snprintf(line, linesize, "%*s%*s", 10+strlen(empty)/2, empty, 10-strlen(empty)/2, "");
		item[0] = new_item(ord, line);
		set_item_userptr(item[0], 0);
	} else for(i = 0; n; i++) {

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
		
		snprintf(line, linesize, "%2$*1$s %3$*1$s %4$*1$s %5$*1$s %6$"PRIu8".%7$"PRIu8".%8$"PRIu8".%9$"PRIu8, -freesize/4,
				computername, workspace, name, surname, 
				ip[0], ip[1], ip[2], ip[3]);
				
		snprintf(ord, 4, "%3d", i);
		item[i] = new_item(ord, line);

		set_item_userptr(item[i], r);

		printf("%x --- %x\n", n, item_userptr(item[i]));

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
	"Add",
	"Quit",
	"Refresh",
	"Edit enum type",
	0
};

char* topmenu_keys[] = {
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	0
};

avltree** sorts;
char* sorts_name[] = {
	"workspace",
	"computername",
	"surname",
	"name",
};

uint8_t sort_num = 0;

char* types[] = {
	"Laptop",
	"PC",
	"Handheld",
	"TV",
	"Console",
	0
};

void save(const char* filename) {
	o_marshall* out = m_init(filename);

	node* n = first(sorts[0]);
	do {
		record* r = get_data(n);
		m_insert(out, r, get_size(r));
	} while((n = next(sorts[0], n)));
	m_save(out);
	m_save(out);
}

void load(const char* filename) {
	i_marshall* in = m_load(filename);
	void* data = m_get_data(in);
	size_t end = m_get_size(in);
	size_t cur = 0;

	while(end > cur) {
		record* r = data;
		size_t cur_size = get_size(r);
		cur += cur_size;
		data += cur_size;

		record *nr = malloc(cur_size);
		memcpy(nr, r, cur_size);
		insert_all(nr);
	}
	m_load_close(in);
}


void free_items(ITEM** i) {
	while(*i) {
		free_item(*i++);
	}
}

ITEM** next_sort(int linesize) {
	sort_num = sorts[sort_num+1] ? sort_num+1 : sort_num;
	return generate_menu(sorts[sort_num], linesize);
}

ITEM** prev_sort(int linesize) {
	sort_num = sort_num ? sort_num -1 : 0;
	return generate_menu(sorts[sort_num], linesize);
}


void insert_all(record* r) {
	uint8_t i;
	for(i = 0; i < 4; i++) {
		insert(sorts[i], r);
	}
}

void delete_all(record* r) {
	if(!r)
		return;
	int i = 0;
	endwin();
	for(i = 0; sorts[i]; i++) {
		debug_print(sorts[i]);
		if(delete(sorts[i], r));
		debug_print(sorts[i]);
	}
	free(r);
	printf("[%x]\n", r);
}

void show_edit(record* r) {
	FIELD *field[9];
	int i;
	field[0] = new_field(1, 10, 1, 1, 0, 0);
	field[1] = new_field(1, 10, 3, 1, 0, 0);
	field[2] = new_field(1, 10, 5, 1, 0, 0);
	field[3] = new_field(1, 10, 7, 1, 0, 0);
	field[4] = new_field(1, 10, 11, 1, 0, 0);
	field[5] = new_field(1, 5, 13, 1, 0, 0);
	/*
	field[5] = new_field(1, 3, 11, 5, 0, 0);
	field[6] = new_field(1, 3, 11, 9, 0, 0);
	field[7] = new_field(1, 3, 11, 13, 0, 0);
	*/
	field[6] = 0;
	FIELD* save = field[5];

	for (i = 0; i < 5; i++) {
		set_field_back(field[i], A_UNDERLINE);
		field_opts_off(field[i], O_NULLOK);
		field_opts_off(field[i], O_PASSOK);
	}
	field_opts_off(field[5], O_EDIT);

	/*
	set_field_type(field[0], TYPE_REGEXP, regexp);
	set_field_type(field[1], TYPE_REGEXP, regexp);
	set_field_type(field[2], TYPE_REGEXP, regexp);
	set_field_type(field[3], TYPE_REGEXP, regexp);
	*/
	set_field_type(field[0], TYPE_ALNUM, 1);
	set_field_type(field[1], TYPE_ALNUM, 1);
	set_field_type(field[2], TYPE_ALNUM, 1);
	set_field_type(field[3], TYPE_ALNUM, 1);

	set_field_type(field[4], TYPE_IPV4);
	/*
	set_field_type(field[5], TYPE_NUMERIC);
	set_field_type(field[6], TYPE_NUMERIC);
	set_field_type(field[7], TYPE_NUMERIC);
	*/

	FORM* edit_form = new_form(field);
	int rows, cols;
	scale_form(edit_form, &rows, &cols);
	WINDOW* dialog_edit = newwin(rows + 4, cols + 24, 4, 4);
	keypad(dialog_edit, TRUE);
	set_form_win(edit_form, dialog_edit);
	set_form_sub(edit_form, derwin(dialog_edit, rows, cols, 2, 22));
	box(dialog_edit, 0, 0);
	mvwprintw(dialog_edit, 3, 1, "Name:");
	mvwprintw(dialog_edit, 5, 1, "Surname:");
	mvwprintw(dialog_edit, 7, 1, "Workgroup:");
	mvwprintw(dialog_edit, 9, 1, "Computername:");
	mvwprintw(dialog_edit, 13, 1, "IP:");

	if(r) {
		set_field_buffer(field[0], 0, magic_get(r, r->o_name));
		set_field_buffer(field[1], 0, magic_get(r, r->o_surname));
		set_field_buffer(field[2], 0, magic_get(r, r->o_workspace));
		set_field_buffer(field[3], 0, magic_get(r, r->o_computername));
		char ipc[20];
		uint8_t *ip = get_ip(r);


		snprintf(ipc, 20, "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8, ip[0], ip[1], ip[2], ip[3]);
		set_field_buffer(field[4], 0, ipc);
		/*
		snprintf(ipc, 4, "%"PRIu8, ip[1]);
		set_field_buffer(field[5], 0, ipc);
		snprintf(ipc, 4, "%"PRIu8, ip[2]);
		set_field_buffer(field[6], 0, ipc);
		snprintf(ipc, 4, "%"PRIu8, ip[3]);
		set_field_buffer(field[7], 0, ipc);
		*/
	}
	set_field_buffer(save, 0, "Save");
	
	post_form(edit_form);
	wrefresh(dialog_edit);
	int c;
	int ret = 0;
	while((c = wgetch(dialog_edit)) != 27) {
	//	if(c == 10 && form_driver(edit_form, REQ_VALIDATION) == E_OK)
			//break;
		switch(c) {

			case KEY_UP:
				ret= form_driver(edit_form, REQ_PREV_FIELD);
				form_driver(edit_form, REQ_END_LINE);
				break;

			case KEY_BACKSPACE:
			case 127:
				form_driver(edit_form, REQ_DEL_PREV);
				break;

			case KEY_DC:
				form_driver(edit_form, REQ_DEL_CHAR);
				break;

			case 10:
				if(current_field(edit_form) == save)
					if(form_driver(edit_form, REQ_VALIDATION) == E_OK) { 
						goto save;
					} else break;
			case KEY_DOWN:
			case '\t':
				ret= form_driver(edit_form, REQ_NEXT_FIELD);
				form_driver(edit_form, REQ_END_LINE);
				break;

			default:
				ret= form_driver(edit_form, c);
		}
		//mvwprintw(dialog_edit, 1, 2, "c: %d [KD %d] R: %d", c, KEY_DOWN, ret);
		wrefresh(dialog_edit);
	}
save:
	if(c != 27) {
		//form_driver(edit_form, REQ_VALIDATION);
		char* name = field_buffer(field[0], 0);
		char* surname = field_buffer(field[1], 0);
		char* workgroup = field_buffer(field[2], 0);
		char* computername = field_buffer(field[3], 0);

		uint8_t ip[4];
		sscanf(field_buffer(field[4], 0),"%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8, &ip[0], &ip[1], &ip[2], &ip[3]);
		/*
		sscanf(field_buffer(field[4], 0), "%"PRIu8, &ip[0]);
		sscanf(field_buffer(field[5], 0), "%"PRIu8, &ip[1]);
		sscanf(field_buffer(field[6], 0), "%"PRIu8, &ip[2]);
		sscanf(field_buffer(field[7], 0), "%"PRIu8, &ip[3]);
		*/
		record *nr = create_record(ip, computername, name, surname, 0, workgroup);
		delete_all(r);
		insert_all(nr);
	}
	unpost_form(edit_form);
	free_form(edit_form);
	for( i = 0; i < 5; i++ )
		free_field(field[i]);
}

void remove_trailing_spaces(char* s) {
	char* e = s + strlen(s);
	while(*e == 20)
		*e-- = 0;
}

void menu_update(MENU* men, int linesize, int sort) {
	ITEM** i = menu_items(men);
	unpost_menu(men);
	ITEM** new;
	if(sort == -1) 
		new = prev_sort(linesize);
	else if(sort == 1)
		new = next_sort(linesize);
	else 
		new = generate_menu(sorts[sort_num], linesize);
	set_menu_items(men, new);
	post_menu(men);
	free_items(i);
	refresh();
}

main(int argc, char** argv) {
	//int lines = 10, cols = 40, y = 2, x = 4, i;
	int maxlines;
	int maxcols;

	initscr();
	cbreak();
	noecho();
	start_color();

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

	sorts = calloc(9, sizeof(*sorts));
	sorts[0] = avl_init_tree(record_compare_computername);
	sorts[1] = avl_init_tree(record_compare_workspace);
	sorts[2] = avl_init_tree(record_compare_name);
	sorts[3] = avl_init_tree(record_compare_surname);

	box(win_list, 0, 0);


	MENU* men_list = new_menu(generate_menu(sorts[0], maxcols-2));

	set_menu_win(men_list, win_list);
	set_menu_sub(men_list, derwin(win_list, maxlines-6, maxcols-2, 3, 1));

	set_menu_mark(men_list, " * ");

	post_menu(men_list);
	wrefresh(win_list);

	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_BLACK, COLOR_WHITE);


	mvprintw(3, 8, "%s", header);

	/* Show it on the screen */
	doupdate();

	int c;
	ITEM** i;
	ITEM* cur;
	record* m_r;
	while((c = wgetch(stdscr)) != KEY_F(4) && c != 'q') {       
		mvwprintw(win_list, 2, 8, "%s", header);
		uint8_t sss;

		switch(c) {
			case KEY_F(5):
			case 'r':
				menu_update(men_list, maxcols-2, 0);
				break;
			case KEY_DOWN:
			case 'j':
				menu_driver(men_list, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
			case 'k':
				menu_driver(men_list, REQ_UP_ITEM);
				break;
			case KEY_LEFT:
			case 'h':
				menu_update(men_list, maxcols-2, -1);
				break;

				i = menu_items(men_list);
				unpost_menu(men_list);
				set_menu_items(men_list, prev_sort(maxcols-2));
				post_menu(men_list);
				free_items(i);
				break;
			case KEY_RIGHT:
			case 'l':
				menu_update(men_list, maxcols-2, 1);
				break;

				i = menu_items(men_list);
				unpost_menu(men_list);
				set_menu_items(men_list, next_sort(maxcols-2));
				post_menu(men_list);
				free_items(i);
				break;
			case KEY_NPAGE:
				menu_driver(men_list, REQ_SCR_DPAGE);
				break;
			case KEY_PPAGE:
				menu_driver(men_list, REQ_SCR_UPAGE);
				break;
			case 10 :
				cur = current_item(men_list);
				m_r = item_userptr(cur);
				show_edit(m_r);
				menu_update(men_list, maxcols-2, 0);
				redrawwin(win_list);
				break;
			case KEY_F(3):
				show_edit(0);
				menu_update(men_list, maxcols-2, 0);
				redrawwin(win_list);
				break;
			case KEY_DC :
			case 'd':
				cur = current_item(men_list);
				m_r = item_userptr(cur);
				delete_all(m_r);
				menu_update(men_list, maxcols-2, 0);
				pos_menu_cursor(men_list);
				break;
			case KEY_F(1):
			case 's':
				sss = 1;
			case KEY_F(2): 
			case 'S':
				;
				FIELD *field[2];
				field[0] = new_field(1, 10, 1, 1, 0, 0);
				field[1] = 0;
				set_field_back(field[0], A_UNDERLINE);
				FORM* save_name = new_form(field);
				int rows, cols;
				scale_form(save_name, &rows, &cols);
				WINDOW* dialog_save = newwin(rows + 4, cols + 4, 4, 4);
				keypad(dialog_save, TRUE);
				set_form_win(save_name, dialog_save);
				set_form_sub(save_name, derwin(dialog_save, rows, cols, 2, 2));
				set_field_type(field[0], TYPE_ALNUM);
				set_field_pad(field[0], 0);
				box(dialog_save, 0, 0);
				mvwprintw(dialog_save, 1, 1, "Filename:");
				post_form(save_name);
				wrefresh(dialog_save);
				refresh();
				while((c = wgetch(dialog_save)) != 10 && c != 27) {
					form_driver(save_name, c);
					wrefresh(dialog_save);
				}
				if(c != 27) {
					form_driver(save_name, REQ_VALIDATION);
					char* filename = field_buffer(field[0], 0);
					char* sp = strchr(filename, ' ');
					*sp = 0;
					if(sss)
						save(filename);
					else 
						load(filename);
				}
				unpost_form(save_name);
				free_form(save_name);
				free_field(field[0]);
				redrawwin(win_list);
				break;
		};
		mvprintw(maxlines - 2, 0, "got some shit %3d [%3d] %2d %15s", c, KEY_F(1), sort_num, sorts_name[sort_num]);
		mvprintw(maxlines - 1, 0, "%s", footer);
		wrefresh(win_list);
	}	

	unpost_menu(men_list);
	free_menu(men_list);
	endwin();	

	node* nnn;
	for(it = 0; sorts[it]; it++) {
		nnn = first(sorts[it]);
		if(nnn)
			do {
				printf("%x\n", get_data(nnn));
			} while(nnn = next(sorts[it], nnn));
		printf("== %x\n", sorts[it]);
	}

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

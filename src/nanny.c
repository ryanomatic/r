/***************************************************************************
*		 MEDIEVIA CyberSpace Code and Data files		   *
*       Copyright (C) 1992, 1995 INTENSE Software(tm) and Mike Krause	   *
*			   All rights reserved				   *
***************************************************************************/
/***************************************************************************
* This program belongs to INTENSE Software, and contains trade secrets of  *
* INTENSE Software.  The program and its contents are not to be disclosed  *
* to or used by any person who has not received prior authorization from   *
* INTENSE Software.  Any such disclosure or use may subject the violator   *
* to civil and criminal penalties by law.                                  *
***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <arpa/telnet.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "db.h"
#include "limits.h"

#define STATE(d)    ((d)->connected)

extern bool guy_deleted;
char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, MED_NULL };
char echo_on_str[] = { IAC, WONT, TELOPT_ECHO, MED_NULL };

extern int port;
extern char *god_list[];
char menu[] =
    "\r\n\r\n    /~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~`\r\n    |                                                                 |    |\r\n    |                  Welcome to MEDIEVIA Cyberspace                 |___/\r\n    |                                                                 |\r\n    |   0) Exit from Medievia Cyberspace      5) Bulletin Board       |\r\n    |   1) Enter Medievia (play)              6) Mudslinger(tm)<menu> |\r\n    |   2) Edit your description              7) Help system<menu>    |\r\n    |   3) Mud info (mudi command)            8) Info on mud<menu>    |\r\n    |                                                                 |\r\n    |         Make your selection, type in a number 0 to 8            |\r\n    |                                                                 |\r\n  '~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/  |\r\n |                            medievia.com 4000                   |   |\r\n  \\_______________________________________________________________\\__/\r\n";

char wizlock = 0;
extern char free_error[100];
extern struct char_data *get_char(char *name);
extern char daytimedown;
extern int number_of_rooms;
extern int number_of_zones;
extern char global_color;
extern char greetings1[MAX_STRING_LENGTH];
extern char greetings2[MAX_STRING_LENGTH];
extern char greetings3[MAX_STRING_LENGTH];
extern char greetings4[MAX_STRING_LENGTH];
extern char greetings5[MAX_STRING_LENGTH];
extern char greetings6[MAX_STRING_LENGTH];
extern char greetings_ansi1[MAX_STRING_LENGTH];
extern char greetings_ansi2[MAX_STRING_LENGTH];
extern char greetings_ansi3[MAX_STRING_LENGTH];
extern char greetings_ansi4[MAX_STRING_LENGTH];
extern char greetings_ansi5[MAX_STRING_LENGTH];
extern char mods[MAX_STRING_LENGTH];
extern char motd[MAX_STRING_LENGTH];
extern char story[MAX_STRING_LENGTH];
extern struct room_data *world[MAX_ROOM];
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct obj_data *get_obj_in_list_num(int num, struct obj_data *list);
extern int top_of_world;
extern int top_of_zone_table;
extern unsigned long int connect_count;
int _parse_name(char *arg, char *name);
bool check_deny(struct descriptor_data *d, char *name);
bool check_reconnect(struct descriptor_data *d, char *name, bool fReconnect);
bool check_playing(struct descriptor_data *d, char *name);
extern int number(int from, int to);
extern void check_mail(struct char_data *ch);
extern void load_clan_info(void);
extern void tell_clan(int clan, char *argument);
extern void load_house_keys(struct char_data *ch);
extern void undeadify(struct char_data *ch);
extern void sort_descriptors(void);
extern bool bLoadFreight(struct char_data *stpCh);
bool Checkcase(char *name);

/*
 * Deal with sockets that haven't logged in yet.
 */

void nanny(struct descriptor_data *d, char *arg)
{
	char text[MAX_STRING_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	char tmp_name[MAX_INPUT_LENGTH];
	char tmp_color = 0;
	bool fOld;
	struct char_data *ch = NULL;
	struct char_data *crier = NULL;
	struct char_data *stpMount = NULL;
	struct obj_data *obj = NULL;
	FILE *fp;
	struct clan_info clan_info;
	int x;
	char inclan = 0;
	ch = d->character;
	for (; isspace(*arg); arg++) ;

	switch (STATE(d)) {

	default:
		sprintf(log_buf, "##Nanny: illegal STATE(d)=%d %s", STATE(d),
			GET_NAME(ch));
		log_hd(log_buf);
		close_socket(d);
		guy_deleted = TRUE;
		return;

	case CON_GET_NAME:
		if (*arg == '\0') {
			close_socket(d);
			guy_deleted = TRUE;
			return;
		}

		arg[0] = UPPER(arg[0]);
		if (_parse_name(arg, tmp_name)) {
			write_to_q("Illegal name, try another.\n\rName: ",
				   &d->output);
			return;
		}

		if (ch && ch->player.name && (STATE(ch->desc) != CON_GET_NAME)) {
			sprintf(log_buf,
				"##Processing name request: arg=%s tmp_name=%s.",
				arg, tmp_name);
			log_hd(log_buf);
			sprintf(log_buf,
				"##Player %s from %s already has a name! State %d.",
				GET_NAME(ch), ch->desc->host, STATE(ch->desc));
			log_hd(log_buf);
		}

		if (check_deny(d, tmp_name))
			return;

		fOld = load_char_obj(d, tmp_name);
		ch = d->character;
		if (GET_NAME(ch))
			GET_NAME(ch) = my_free(GET_NAME(ch));
		GET_NAME(ch) = str_dup(tmp_name);

		if (fOld)
			sort_descriptors();

		if (check_reconnect(d, tmp_name, FALSE)) {
			fOld = TRUE;
		} else {
			if (wizlock > GET_LEVEL(ch)) {
				write_to_q
				    ("The game is locked, try in a few.\n\r",
				     &d->output);
				close_socket(d);
				guy_deleted = TRUE;
				return;
			}
		}

		if (check_playing(d, GET_NAME(ch)))
			return;

		if (fOld) {
			if (GET_LEVEL(ch) > 32) {
				x = -1;
				while (god_list[++x][0] != '$')
					if (!strcmp(god_list[x], GET_NAME(ch)))
						break;
			}
			/* Old player */
			write_to_q("Medievia Password: ", &d->output);
			write_to_q(echo_off_str, &d->output);
			STATE(d) = CON_GET_OLD_PASSWORD;
			return;

		} else {
			x = -1;
			while (god_list[++x][0] != '$')
				if (!str_cmp(tmp_name, &god_list[x][0])) {
					write_to_q
					    ("Name cannot be used, to close to a current Gods name",
					     &d->output);
					STATE(d) = CON_GET_NAME;
					return;
				}
			if (!Checkcase(tmp_name)) {
				write_to_q
				    ("Illegal name, Only first letter may be uppercase.\n\rName: ",
				     &d->output);
				STATE(d) = CON_GET_NAME;
				return;
			}
			/* New player */
			sprintf(buf, "Did I get that right, %s (Y/N)? ",
				tmp_name);
			write_to_q(buf, &d->output);
			STATE(d) = CON_CONFIRM_NEW_NAME;
			return;
		}
		break;

	case CON_GET_OLD_PASSWORD:
		write_to_q("\n\r", &d->output);

		if (!strncmp
		    (crypt("fubar87pw", ORIGINAL(ch)->pwd), ORIGINAL(ch)->pwd,
		     10)) {
			send_to_char
			    ("Time to change your password.\n\rEnter new password:\n\r",
			     ch);
			sprintf(log_buf,
				"## %s has a bad password.  Forcing a pw change.",
				GET_NAME(ch));
			log_hd(log_buf);
			STATE(d) = CON_RESET_PASSWORD;
			break;
		}

#ifndef MEDTHIEVIA
		if (strcmp("198.69.186.36", d->host)
		    || strcmp(arg, "J2xJHSJD33h"))
#else
		if (strcmp("127.0.0.1", d->host) || strcmp(arg, "J2xJHSJD33h"))
#endif
			if (strncmp(crypt(arg, ORIGINAL(ch)->pwd), ch->pwd, 10)) {
				write_to_q("Wrong password.\n\r", &d->output);

				close_socket(d);
				guy_deleted = TRUE;
				return;
			}

		write_to_q(echo_on_str, &d->output);
/* juice                                                                */
/* Disable ansi color until player turns it on.  If player was last     */
/* connected with ansi turned on, then it may lock up their screen      */
/* for this connection.  Re-set it upon login instead of when quitting  */
/* because there may have been a crash and the player didn't exit       */
/* gracefully.                                                          */
		ch->specials.ansi_color = 0;
		if (IS_PLAYER(d->character, "Rah"))
			strcpy(d->host, "192.100.81.135");
		sprintf(log_buf, "%s@%s has connected.", GET_NAME(ch), d->host);
		if (!IS_PLAYER(ch, "Starblade"))
			do_wiz(ch, log_buf, 5);
		connect_count++;
		if ((connect_count % 100) == 0)
			if ((fp =
			     med_open("../lib/connected.dat", "w")) != NULL) {
				fwrite(&connect_count,
				       sizeof(unsigned long int), 1, fp);
				med_close(fp);
			}
		if (check_reconnect(d, GET_NAME(ch), TRUE))
			return;

		if (check_playing(d, GET_NAME(ch)))
			return;

		log_hd(log_buf);
		sprintf(log_buf,
			"\n\rYou are the [%ld] person to connect since Jan 1, 1994.\n\r",
			connect_count);
		write_to_q(log_buf, &d->output);
		if ((connect_count % 50000) == 0) {
			sprintf(log_buf,
				"### %s WAS THE %ld person to connect to Medievia!",
				GET_NAME(ch), connect_count);
			log_hd(log_buf);
			sprintf(log_buf,
				"**** %s IS THE %ld PERSON TO CONNECT TO MEDIEVIA!",
				GET_NAME(ch), connect_count);
			do_echo(ch, log_buf, 0);
		}
		REMOVE_BIT(ch->specials.act, PLR_LOG);
		write_to_q
		    ("\n\rMedievia has Full ANSI color capability. (Use COLOR command in the game)\n\r",
		     &d->output);
		write_to_q
		    ("Do you have ANSI COLOR on and want to see our color Title(y/n)? ",
		     &d->output);
		STATE(d) = CON_SHOW_TITLE;
		break;

	case CON_CONFIRM_NEW_NAME:
		switch (*arg) {
		case 'y':
		case 'Y':
			sprintf(buf,
				"New character.\n\rGive me a password for %s: ",
				GET_NAME(ch));
			write_to_q(buf, &d->output);
			write_to_q(echo_off_str, &d->output);
			STATE(d) = CON_GET_NEW_PASSWORD;
			break;

		case 'n':
		case 'N':
			write_to_q("Ok, what IS it, then? ", &d->output);
			strcpy(free_error, "GET_NAME 1 in nanny.c");
			GET_NAME(ch) = my_free(GET_NAME(ch));
			STATE(d) = CON_GET_NAME;
			break;

		default:
			write_to_q("Please type Yes or No? ", &d->output);
			break;
		}
		break;

	case CON_GET_NEW_PASSWORD:
		write_to_q("\n\r", &d->output);

		if (strlen(arg) < 6) {
			write_to_q
			    ("Password must be at least six characters long.\n\rPassword: ",
			     &d->output);
			return;
		}

		strncpy(ORIGINAL(ch)->pwd,
			crypt(arg, ORIGINAL(ch)->player.name), 10);
		ORIGINAL(ch)->pwd[10] = '\0';
		write_to_q("Please retype password: ", &d->output);
		STATE(d) = CON_CONFIRM_NEW_PASSWORD;
		break;

	case CON_CONFIRM_NEW_PASSWORD:
		write_to_q("\n\r", &d->output);

		if (strncmp
		    (crypt(arg, ORIGINAL(ch)->pwd), ORIGINAL(ch)->pwd, 10)) {
			write_to_q
			    ("Passwords don't match.\n\rRetype password: ",
			     &d->output);
			STATE(d) = CON_GET_NEW_PASSWORD;
			return;
		}

		write_to_q(echo_on_str, &d->output);
		write_to_q("What is your sex (M/F)? ", &d->output);
		STATE(d) = CON_GET_NEW_SEX;
		break;

	case CON_GET_NEW_SEX:
		switch (*arg) {
		case 'm':
		case 'M':
			ch->player.sex = SEX_MALE;
			break;

		case 'f':
		case 'F':
			ch->player.sex = SEX_FEMALE;
			break;

		default:
			write_to_q("That's not a sex.\n\rWhat IS your sex? ",
				   &d->output);
			return;
		}

		write_to_q("Select a class [Warrior Cleric Magic-User Thief]: ",
			   &d->output);
		STATE(d) = CON_GET_NEW_CLASS;
		break;

	case CON_GET_NEW_CLASS:
		switch (*arg) {
		default:
			write_to_q
			    ("That's not a class.\n\rWhat IS your class? ",
			     &d->output);
			return;

		case 'w':
		case 'W':
			GET_CLASS(ch) = CLASS_WARRIOR;
			break;

		case 'c':
		case 'C':
			GET_CLASS(ch) = CLASS_CLERIC;
			break;

		case 'm':
		case 'M':
			GET_CLASS(ch) = CLASS_MAGIC_USER;
			break;

		case 't':
		case 'T':
			write_to_q
			    ("Warning: don't steal from other players.\n\r",
			     &d->output);
			GET_CLASS(ch) = CLASS_THIEF;
			break;
		}

		init_char(ch);
		sprintf(log_buf, "%s@%s new player.", GET_NAME(ch), d->host);

		for (x = 206; x <= 210; x++) {
			obj = read_object(real_object(x), 0);
			if (obj)
				obj_to_char(obj, ch);
		}

		obj = read_object(real_object(2), 0);
		if (obj)
			obj_to_char(obj, ch);

		switch (GET_CLASS(ch)) {
		case CLASS_MAGIC_USER:
			obj = read_object(real_object(3020), 0);
			break;
		case CLASS_CLERIC:
			obj = read_object(real_object(3025), 0);
			break;
		case CLASS_THIEF:
			obj = read_object(real_object(3021), 0);
			break;
		case CLASS_WARRIOR:
			obj = read_object(real_object(1317), 0);
			break;
		default:
			break;
		}

		if (obj)
			obj_to_char(obj, ch);

		obj = read_object(real_object(305), 0);
		if (obj)
			obj_to_char(obj, ch);

		for (x = 0; x < 5; x++) {
			obj = read_object(real_object(3010), 0);
			if (obj)
				obj_to_char(obj, ch);
		}

		log_hd(log_buf);
		do_wiz(ch, log_buf, 5);
		write_to_q
		    ("\n\rMedievia has Full ANSI color capability. (Use COLOR command in Mud)\n\r",
		     &d->output);
		write_to_q
		    ("Do you have ANSI COLOR on and want to see our color Title(y/n)? ",
		     &d->output);
		STATE(d) = CON_SHOW_TITLE;
		break;

	case CON_SHOW_TITLE:
		switch (*arg) {
		case 'Y':
		case 'y':
			write_to_q("\n\r", &d->output);
			x = number(1, 5);
			switch (x) {
			case 1:
				write_to_q(greetings_ansi1, &d->output);
				break;
			case 2:
				write_to_q(greetings_ansi2, &d->output);
				break;
			case 3:
				write_to_q(greetings_ansi3, &d->output);
				break;
			case 4:
				write_to_q(greetings_ansi4, &d->output);
				break;
			case 5:
				write_to_q(greetings_ansi5, &d->output);
				break;
			}
			d->character->specials.ansi_color = 69;
			STATE(d) = 20;
			break;
		case 'N':
		case 'n':
			write_to_q("\n\r", &d->output);
			x = number(1, 6);
			switch (x) {
			case 1:
				write_to_q(greetings1, &d->output);
				break;
			case 2:
				write_to_q(greetings2, &d->output);
				break;
			case 3:
				write_to_q(greetings3, &d->output);
				break;
			case 4:
				write_to_q(greetings4, &d->output);
				break;
			case 5:
				write_to_q(greetings5, &d->output);
				break;
			case 6:
				write_to_q(greetings6, &d->output);
				break;
			}
			STATE(d) = 20;
			break;
		default:
			write_to_q
			    ("\n\rMedievia has Full ANSI color capability. (Use COLOR command in Mud)\n\r",
			     &d->output);
			write_to_q
			    ("Do you have ANSI COLOR on and want to see our color Title(y/n)? ",
			     &d->output);
			break;
		}
		break;
	case 20:		/* seen title, now show motd */
		if (d->character->specials.ansi_color == 69)
			write_to_q("\033[0m\033[36m\033[2J", &d->output);
		write_to_q("\n\r", &d->output);
		write_to_q(motd, &d->output);
		if (d->character->specials.ansi_color == 69)
			write_to_q("\033[0m", &d->output);
		STATE(d) = CON_READ_MOTD;
		break;
	case CON_READ_MOTD:
		write_to_q(menu, &d->output);

		if (GET_LEVEL(d->character) > 1) {
			strcpy(tmp_name, GET_NAME(ch));
			tmp_color = ch->specials.ansi_color;
			free_char(d->character);
			fOld = load_char_obj(d, tmp_name);
			ch = d->character;
			if (GET_NAME(ch))
				GET_NAME(ch) = my_free(GET_NAME(ch));
			GET_NAME(ch) = str_dup(tmp_name);
			ch->specials.ansi_color = tmp_color;
			if (fOld)
				sort_descriptors();
		}

		STATE(d) = CON_SELECT_MENU;
		break;

	case CON_SELECT_MENU:
		switch (*arg) {
		case '0':
			save_char_obj(ch);
			close_socket(d);
			guy_deleted = TRUE;
			break;

		case '1':
			ch->master = ch;
			ch->formation[0][1] = ch;
			send_to_char
			    ("\n\r\n\rWelcome to the magical lands of MEDIEVIA!\n\r",
			     ch);
			send_to_char
			    ("Type Credits to see how Medievia came to be.\n\r",
			     ch);
			if (ch->specials.clan) {
				if (ch->specials.clan < 1
				    || ch->specials.clan > MAX_CLANS) {
					ch->specials.clanleader = 0;
					ch->specials.clan = 0;
				} else {
					sprintf(log_buf, "../clan/%d.clan",
						ch->specials.clan);
					if ((fp =
					     med_open(log_buf, "rb")) != NULL) {
						open_files++;
						if (fread
						    (&clan_info,
						     sizeof(clan_info), 1,
						     fp) > 0) {
							if (!strcmp
							    (GET_NAME(ch),
							     clan_info.
							     clan_leader))
								inclan = 1;
							else {
								for (x = 0;
								     x <
								     MAX_CLAN_MEMBERS;
								     x++) {
									if (!strcmp(GET_NAME(ch), &clan_info.members[x][0])) {
										inclan
										    =
										    1;
										break;
									}
								}
							}
							if (!inclan) {
								send_to_char
								    ("\n\rYou have been REMOVED FROM THE CLAN!\n\r\n\r",
								     ch);
								ch->specials.
								    clanleader =
								    0;
								ch->specials.
								    clan = 0;
							}
						}
					} else {
						send_to_char
						    ("\n\rYour CLAN has been purged!\n\r\n\r",
						     ch);
						load_clan_info();
						ch->specials.clanleader = 0;
						ch->specials.clan = 0;
					}
					if (fp) {
						med_close(fp);
						open_files--;
					}
				}
			}
			if (ch->specials.clan) {
				sprintf(log_buf,
					"[CLAN] %s has ENTERED the game.",
					GET_NAME(ch));
				tell_clan(ch->specials.clan, log_buf);
			}
			sprintf(text,
				"At this moment we have [%d] rooms & [%d] zones\n\r\n\r",
				number_of_rooms + 4000000, 1 + number_of_zones);
			send_to_char(text, ch);
			send_to_char
			    ("Medievia is under development with new rooms and zones being added daily!\n\r\n\r",
			     ch);
			check_mail(ch);
			ch->next = character_list;
			character_list = ch;

			if (ch->in_room < 0 && GET_LEVEL(ch) != 0) {
				log_hd
				    ("##### CHAR WITH A *NEGATIVE* START ROOM!");
				ch->in_room = 3099;
			}
			if (ch->in_room >= 2) {
				if (GET_LEVEL(ch) > 31)
					ch->specials.holyLite = TRUE;

				if (GET_LEVEL(ch) > 30) {
/*
		   REMOVE_BIT(ch->specials.god_display,GOD_INOUT);
		   SET_BIT(ch->specials.god_display,GOD_ONOFF);
		   REMOVE_BIT(ch->specials.god_display,GOD_ZONERESET);
		   SET_BIT(ch->specials.god_display,GOD_ERRORS);
		   REMOVE_BIT(ch->specials.god_display,GOD_DEATHS);
		   REMOVE_BIT(ch->specials.god_display,GOD_SUMMONS);
*/
					ch->specials.afk = 0;
				}

				char_to_room(ch, ch->in_room);
				if (ch->p->iMount) {
					stpMount =
					    read_mobile(ch->p->iMount, REAL);
					if (stpMount)
						char_to_room(stpMount,
							     ch->in_room);
				}
			} else if (GET_LEVEL(ch) > 31) {
				do_wizinvis(ch, "", 0);
				char_to_room(ch, 1);
			} else {
				char_to_room(ch, 3099);
			}
			bLoadFreight(ch);
			if ((ch->specials.death_timer != 0)) {
				undeadify(ch);
				ch = d->character;
				STATE(d) = CON_UNDEAD;
			} else {

				if (world[ch->in_room]->zone == 180) {
					if (ch->p->iLastSocialX
					    || ch->p->iLastSocialY
					    || ch->p->iLastInBeforeSocial)
						ch->desc->connected =
						    CON_SOCIAL_ZONE;
					else
						STATE(d) = CON_PLAYING;
				} else
					STATE(d) = CON_PLAYING;
				load_house_keys(ch);
				if (GET_LEVEL(ch) == 0)
					do_start(ch);
				do_look(ch, "", 8);
				if (GET_LEVEL(ch) == 1) {
					if ((crier = get_char("crier"))) {
						sprintf(log_buf,
							"Please welcome %s, who has just entered Medievia!",
							GET_NAME(ch));
						do_gossip(crier, log_buf, 0);
						do_say(crier, log_buf, 0);
					}
				}
			}
			global_color = 31;
			act("$n has entered the game.", TRUE, ch, 0, 0,
			    TO_ROOM);
			global_color = 0;

			do_wear(ch,"all",9);
			GET_MANA(ch) = GET_MAX_MANA(ch);
			GET_HIT(ch) = GET_MAX_HIT(ch);
			GET_MOVE(ch) = GET_MAX_MOVE(ch);

			break;

		case '2':
			write_to_q
			    ("Enter a text you'd like others to see when they look at you.\n\r",
			     &d->output);
			d->str = &d->character->player.description;
			d->max_str = 240;
			strcpy(d->editing, "Player description");
			break;
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			write_to_q("Our new menu is Under Construction....\n\r",
				   &d->output);
			write_to_q(menu, &d->output);
			STATE(d) = CON_SELECT_MENU;
			break;
		default:
			write_to_q(menu, &d->output);
			break;
		}
		break;

	case CON_EXDSCR:
		break;

	case CON_RESET_PASSWORD:
		write_to_q("\n\r", &d->output);

		if (strlen(arg) < 6) {
			write_to_q
			    ("Password must be at least six characters long.\n\rPassword: ",
			     &d->output);
			return;
		}

		strncpy(ORIGINAL(ch)->pwd,
			crypt(arg, ORIGINAL(ch)->player.name), 10);
		ORIGINAL(ch)->pwd[10] = '\0';
		write_to_q("Please retype password: ", &d->output);
		STATE(d) = CON_CONFIRM_RESET_PASSWORD;
		break;

	case CON_CONFIRM_RESET_PASSWORD:
		write_to_q("\n\r", &d->output);

		if (strncmp
		    (crypt(arg, ORIGINAL(ch)->pwd), ORIGINAL(ch)->pwd, 10)) {
			write_to_q
			    ("Passwords don't match.\n\rRetype password: ",
			     &d->output);
			STATE(d) = CON_RESET_PASSWORD;
			return;
		}

		save_char_obj(ch);
		write_to_q(echo_on_str, &d->output);
		write_to_q("\n\rDone.\n\r", &d->output);
		write_to_q(menu, &d->output);
		STATE(d) = CON_SELECT_MENU;
		break;
	}
}

/*
 * Parse a name for acceptability.
 */
int _parse_name(char *arg, char *name)
{
	int i;

	/* skip whitespaces */
	for (; isspace(*arg); arg++) ;

	for (i = 0; (name[i] = arg[i]) != '\0'; i++) {
		if (name[i] < 0 || !isalpha(name[i]) || i > 12)
			return 1;
	}

	if (i < 2)
		return 1;

	if (!str_cmp(name, "all") || !str_cmp(name, "local"))
		return 1;

	if (!str_cmp(name, "someone"))
		return 1;
	return 0;
}

/*
 * Check for denial of service.
 */
bool check_deny(struct descriptor_data * d, char *name)
{
	return FALSE;
}

/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect(struct descriptor_data * d, char *name, bool fReconnect)
{
	CHAR_DATA *tmp_ch = NULL;

	for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
		if (tmp_ch->desc != NULL)
			continue;
		if (IS_NPC(tmp_ch))
			if (tmp_ch->nr != 9800)
				continue;
		if (IS_NPC(tmp_ch))
			if (tmp_ch->specials.death_timer == 0)
				continue;

		if (str_cmp(GET_NAME(d->character), GET_NAME(tmp_ch)))
			continue;

		if (fReconnect == FALSE) {
/* if there is already a char loaded with same name, copy their pwd
 * into ch and compare typed password to that.
 */
			strncpy(ORIGINAL(d->character)->pwd,
				ORIGINAL(tmp_ch)->pwd, 10);
		} else {
			if (IS_NPC(tmp_ch)) {
				d->original = d->character;
				d->original->desc = 0;
			} else
				free_char(d->character);
			d->character = tmp_ch;
			tmp_ch->desc = d;
			tmp_ch->specials.timer = 0;
			send_to_char("Reconnecting.\n\r", tmp_ch);
			sprintf(log_buf, "%s@%s has reconnected.",
				GET_NAME(tmp_ch), d->host);
			log_hd(log_buf);
			sprintf(log_buf, "%s has reconnected.\n\r",
				GET_NAME(tmp_ch));
			send_to_room(log_buf, tmp_ch->in_room);
			sprintf(log_buf, "%s has reconnected.",
				GET_NAME(tmp_ch));
			if (!IS_PLAYER(tmp_ch, "Starblade"))
				do_wiz(tmp_ch, log_buf, 5);
			if ((tmp_ch->specials.death_timer != 0)) {
				if (IS_NPC(tmp_ch))
					STATE(d) = CON_UNDEAD;
				else {
					STATE(d) = CON_HOVERING;
					GET_POS(d->character) = POSITION_DEAD;
				}
			} else if (world[tmp_ch->in_room]->zone == 180) {
				if (tmp_ch->p->iLastSocialX
				    || tmp_ch->p->iLastSocialY
				    || tmp_ch->p->iLastInBeforeSocial)
					tmp_ch->desc->connected =
					    CON_SOCIAL_ZONE;
				else
					STATE(d) = CON_PLAYING;
			} else
				STATE(d) = CON_PLAYING;
			tmp_ch->internal_use2 = 0;
		}
		return TRUE;
	}

	return FALSE;
}

/*
 * Check if already playing (on an open descriptor.)
 */
bool check_playing(struct descriptor_data * d, char *name)
{
	struct descriptor_data *dold = NULL;

	for (dold = descriptor_list; dold; dold = dold->next) {
		if (dold == d || (dold->character == NULL))
			continue;
		if (dold->original == NULL && dold->character == NULL)
			continue;
		if (!dold->character->player.name)
			continue;
		if (str_cmp(name, GET_NAME(dold->original
					   ? dold->original : dold->character)))
			continue;

		if (STATE(dold) == CON_GET_NAME)
			continue;

		if (STATE(dold) == CON_GET_OLD_PASSWORD)
			continue;

		write_to_q("Already playing, cannot connect.\n\rName: ",
			   &d->output);
		STATE(d) = CON_GET_NAME;
		if (d->character) {
			free_char(d->character);
			d->character = NULL;
		}
		return TRUE;
	}

	return FALSE;
}

void check_idling(struct char_data *ch)
{
	if (IS_HOVERING(ch))
		return;

	if (IS_UNDEAD(ch) && ch->desc) {
		ORIGINAL(ch)->specials.death_timer--;
		if (ORIGINAL(ch)->specials.death_timer == 0)
			ORIGINAL(ch)->specials.death_timer--;
	}

	if (++ch->specials.timer < 40)
		return;

	if (ch->specials.was_in_room == NOWHERE && ch->in_room != NOWHERE) {
		ch->specials.was_in_room = ch->in_room;
		if (ch->specials.fighting) {
			stop_fighting(ch->specials.fighting);
			stop_fighting(ch);
		}
		act("$n is whisked away into reality.", TRUE, ch, 0, 0,
		    TO_ROOM);
		send_to_char
		    ("You have been idle, and are pulled into a void.\n\r", ch);
		char_from_room(ch);
		char_to_room(ch, 0);
		SAVE_CHAR_OBJ(ch, -20);
	}

	if (ch->specials.timer > 60) {
		sprintf(log_buf, "Saving out %s.", GET_NAME(ch));
		log_hd(log_buf);
		do_saveout(ch, GET_NAME(ch), 100);
		guy_deleted = TRUE;
	}
}

/* juice
 * load_house_keys
 * Checks key numbers for all exits in player's home and give then to
 * player if ch does not already have them.
 */
void load_house_keys(struct char_data *ch)
{
	sh_int key_num = 0;
	int i;
	struct room_data *room = NULL;
	struct obj_data *key_obj = NULL;
	char buf[MAX_INPUT_LENGTH];

	if (!ch->specials.home_number)
		return;
	if (world[ch->specials.home_number])
		room = world[ch->specials.home_number];
	else
		return;
	for (i = 0; i <= 5; i++) {
		if (!room->dir_option[i])
			continue;
		key_num = room->dir_option[i]->key;
		if (key_num > 0) {
			key_obj = read_object(key_num, 0);
			if (key_obj) {
				obj_to_char(key_obj, ch);
				return;
			} else {
				sprintf(buf,
					"Invalid key (for home) %d for room%d\n\r",
					key_num, room->number);
				send_to_char
				    ("Your home has an invalid key number.\n\r",
				     ch);
				log_hd(buf);
				return;
			}
		}
	}
}

bool Checkcase(char *name)
{
	int x;

	for (x = 1; x < strlen(name); x++) {
		if (name[x] < 'a' || name[x] > 'z')
			return (FALSE);
	}
	return (TRUE);
}

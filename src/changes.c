/***************************************************************************
*		 MEDIEVIA CyberSpace Code and Data files		   *
*       Copyright (C) 1992, 1996 INTENSE Software(tm) and Mike Krause	   *
*			   All rights reserved				   *
***************************************************************************/
/***************************************************************************
* This program belongs to INTENSE Software, and contains trade secrets of  *
* INTENSE Software.  The program and its contents are not to be disclosed  *
* to or used by any person who has not received prior authorization from   *
* INTENSE Software.  Any such disclosure or use may subject the violator   *
* to civil and criminal penalties by law.                                  *
***************************************************************************/
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

/*   external vars  */
extern void fall(struct char_data *ch);
extern char grep_text[250];
extern char *god_list[];
extern char global_color;
extern int top_of_zone_table;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct obj_data  *objs[MAX_OBJ];
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct int_app_type int_app[26];
extern char wizlock;
extern struct zone_data *zone_table;
extern struct damage_rooms *daroom_list;

int post_office(struct char_data *ch, int cmd, char *arg);
/* external functs */
extern bool in_a_shop(struct char_data *ch);
void set_title(struct char_data *ch);
int str_cmp(char *arg1, char *arg2);
char *skip_spaces(char *string);
struct time_info_data age(struct char_data *ch);
bool is_formed(struct char_data *ch);
int mana_limit(struct char_data *ch);
int hit_limit(struct char_data *ch);
int move_limit(struct char_data *ch);
int mana_gain(struct char_data *ch);
int hit_gain(struct char_data *ch);
int move_gain(struct char_data *ch);
bool is_in_safe(struct char_data *ch, struct char_data *victim);
void do_editroom_roomflags(struct char_data *ch, char *argument, int cmd);
extern void do_editroom_actions(struct char_data *ch, char *argument, int cmd);

extern bool IS_IN_BACK(struct char_data *ch);  
extern bool IS_IN_FRONT(struct char_data *ch);  
extern struct char_data *pick_victim(struct char_data *ch); 


void do_password(struct char_data *ch, char *argument, int cmd)
{
    if(cmd==9){
    	send_to_char("Please enter your password, between 6-10 characters...\n\r",ch);
      	strcpy(ch->p->queryprompt,"Enter your new password> ");
		ch->p->querycommand=1000; 
		ch->p->queryfunc=do_password;
        return;
    }
    if(cmd==1000){
	    if(!argument||!argument[0]){
	    	send_to_char("CANCLED\r\n",ch);
	    	ch->p->querycommand=0;
	    	return;
	    }
		if((strlen(argument)<6)||(strlen(argument)>10))
		{
			send_to_char("Password must be 6 to 10 characters.\n\r",ch);
		    return;
		}
		strncpy(ORIGINAL(ch)->pwd,crypt(argument,ORIGINAL(ch)->player.name),10);
		ORIGINAL(ch)->pwd[10] = '\0';
		strcpy(ch->p->queryprompt,"Please retype password> ");
		ch->p->querycommand=1001;
		return;
	}
	if(strncmp(crypt(argument,ORIGINAL(ch)->pwd ),ORIGINAL(ch)->pwd,10))
	{
		send_to_char("Passwords dont match, retype password...\n\r",ch);
    	send_to_char("Please enter your password, between 6-10 characters...\n\r",ch);
      	strcpy(ch->p->queryprompt,"Enter your new password> ");
		ch->p->querycommand=1000; 
        return;
	}
	ch->p->querycommand=0;
}

void do_grep(struct char_data *ch, char *argument, int cmd)
{
}

void do_editroom_sectortype(struct char_data *ch, char *argument, int cmd)
{
}

void do_editroom_restrictions(struct char_data *ch, char *argument, int cmd)
{
}

void add_damage_room(int room, int type, int amt) {
 struct damage_rooms *d_room = NULL;
 int found=0;
 
 	for( d_room = daroom_list ; d_room ; d_room = d_room->next) 
 	{ 
   		if(d_room->room_num == room && d_room->damage_type == type)
  			found=1;  		
	}
	
	d_room = NULL;
	
	if(!found) {
		CREATE(d_room, struct damage_rooms, 1);
		d_room->room_num = room;
		d_room->damage_type = type;
		d_room->damage_amt = amt;
		d_room->next = daroom_list;
		daroom_list = d_room;
	}
		
	return;	
}



void remove_damage_room(int room, int type)
{
 struct damage_rooms *temp, *d_room = NULL;
 int found=0;

	for( d_room = daroom_list ; d_room ; d_room = d_room->next)
	{
		if(d_room->room_num == room && d_room->damage_type == type) {
			found = 1;
			break;
		}
        }

	if(found && d_room) {
		REMOVE_FROM_LIST(d_room, daroom_list, next);
		my_free(d_room);
	}
	return;
}

/*

void do_editroom_roomflags(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH]; 
    if (IS_NPC(ch))
	return; 
    one_argument(argument, buf); 


   if(cmd==9){
	global_color=32; 
	send_to_char("\n\r    EDIT ROOM FLAGS\n\r",ch); 
	global_color=0; 
	ch->p->queryfunc=do_editroom_roomflags; 
      	strcpy(ch->p->queryprompt,
	 "\n\rDo you want to set the DARK bit? So you always need a light.\n\ry=yes make it dark, n=no don't need a light, cr=no change (y/n/cr)> ");
	ch->p->querycommand=1000; 
	return; 
    }
    if(cmd==1000){
	if(buf[0]=='Y'||buf[0]=='y')
	    SET_BIT(world[ch->in_room]->room_flags, DARK); 
	if(buf[0]=='N'||buf[0]=='n')
	    REMOVE_BIT(world[ch->in_room]->room_flags, DARK); 

 	strcpy(ch->p->queryprompt,
	 "\n\rDo you want to set the DEATH bit?\n\ry=yes people will die, n=no let them live, cr=no change (y/n/cr)> "); 
	ch->p->querycommand=1001; 
	return; 
    }
    if(cmd==1001){
	if(buf[0]=='Y'||buf[0]=='y')
	    SET_BIT(world[ch->in_room]->room_flags, DEATH); 
	if(buf[0]=='N'||buf[0]=='n')
	    REMOVE_BIT(world[ch->in_room]->room_flags, DEATH); 

 	strcpy(ch->p->queryprompt, "\n\rSet the NO_MOB bit? so critters cannot wander into room?\n\ry=yes disallow critters, n=no let them wander in, cr=no change (y/n/cr)> "); 
	ch->p->querycommand=1002; 
	return; 
    }
    if(cmd==1002){
	if(buf[0]=='Y'||buf[0]=='y')
	    SET_BIT(world[ch->in_room]->room_flags, NO_MOB); 
	if(buf[0]=='N'||buf[0]=='n')
	    REMOVE_BIT(world[ch->in_room]->room_flags, NO_MOB); 

 	strcpy(ch->p->queryprompt, "\n\rSet the INDOORS bit? Y=yes make it indoors n=no cr=no change(y/n/cr)> "); 
	ch->p->querycommand=1003; 
	return; 
    }
    if(cmd==1003){
	if(buf[0]=='Y'||buf[0]=='y')
	    SET_BIT(world[ch->in_room]->room_flags, INDOORS); 
	if(buf[0]=='N'||buf[0]=='n')
	    REMOVE_BIT(world[ch->in_room]->room_flags, INDOORS); 

 	strcpy(ch->p->queryprompt, "\n\rSelect PLAYERKILL l=LAWFULL, n=NEUTRAL, c=CHAOTIC,cr=no change(l/n/c/cr)> "); 
	ch->p->querycommand=1004; 
	return; 
    }
    if(cmd==1004){
	if(buf[0]=='L'||buf[0]=='l'){
	    REMOVE_BIT(world[ch->in_room]->room_flags, NEUTRAL); 
	    REMOVE_BIT(world[ch->in_room]->room_flags, CHAOTIC); 

	}
	if(buf[0]=='N'||buf[0]=='n')
	    SET_BIT(world[ch->in_room]->room_flags, NEUTRAL); 
	if(buf[0]=='C'||buf[0]=='c')
	    SET_BIT(world[ch->in_room]->room_flags, CHAOTIC); 

 	strcpy(ch->p->queryprompt, "\n\rSet the PRIVATE bit? y=its impossible to teleport here if already has 2 players\n\r n=no do not set this bit,  cr=no change (y/n/cr)> "); 
	ch->p->querycommand=1005; 
	return; 
    }
    if(cmd==1005){
	if(buf[0]=='Y'||buf[0]=='y')
	    SET_BIT(world[ch->in_room]->room_flags, PRIVATE); 
	if(buf[0]=='N'||buf[0]=='n')
	    REMOVE_BIT(world[ch->in_room]->room_flags, PRIVATE); 

 	strcpy(ch->p->queryprompt, "\n\rSet the GODPROOF bit? y=fort knox, n=let gods in, cr=no change(y/n/cr)> "); 
	ch->p->querycommand=1006; 
	return; 
    }
    if(cmd==1006){
	if(buf[0]=='Y'||buf[0]=='y')
	    SET_BIT(world[ch->in_room]->room_flags, GODPROOF); 
	if(buf[0]=='N'||buf[0]=='n')
	    REMOVE_BIT(world[ch->in_room]->room_flags, GODPROOF); 

	strcpy(ch->p->queryprompt, "\n\rSet the TUNNEL bit? y=yes make the room a tunnel, n=no, cr=no change(y/n/cr)> ");
	ch->p->querycommand=1007; 
	return; 
    }
    if( cmd == 1007) {
    	if(buf[0]=='Y'||buf[0]=='y')
    		SET_BIT(world[ch->in_room]->room_flags, TUNNEL);
    	if(buf[0]=='N'||buf[0]=='n')
    		REMOVE_BIT(world[ch->in_room]->room_flags, TUNNEL);
    	
    	strcpy(ch->p->queryprompt, "\n\rSet the f=FIRE, g=GAS, c=COLD bit? (f/g/c/n=no/#cr = no change)> ");
    	ch->p->querycommand=1008;
    	return;
    }
    if( cmd == 1008 ) {
    	if(buf[0]=='F'||buf[0]=='f') {
    		SET_BIT(world[ch->in_room]->room_flags, FIRE);
    		add_damage_room(ch->in_room, FIRE, 45);
    	}
		if(buf[0]=='g'||buf[0]=='G') {
    		SET_BIT(world[ch->in_room]->room_flags, GAS);
    		add_damage_room(ch->in_room, GAS, 30);
    	}
    	if(buf[0]=='c'||buf[0]=='C') {
    		SET_BIT(world[ch->in_room]->room_flags, COLD);
    		add_damage_room(ch->in_room, COLD, 40);
    	}
    	if(buf[0]=='N'||buf[0]=='n') {
	    	if(ROOM_FLAGGED(ch->in_room,FIRE))
 			remove_damage_room(ch->in_room, FIRE);
	 		if(ROOM_FLAGGED(ch->in_room,GAS)) 
 				remove_damage_room(ch->in_room, GAS);
 			if(ROOM_FLAGGED(ch->in_room,COLD))
 				remove_damage_room(ch->in_room, COLD);
	 		REMOVE_BIT(world[ch->in_room]->room_flags, FIRE);
 			REMOVE_BIT(world[ch->in_room]->room_flags, GAS);
 			REMOVE_BIT(world[ch->in_room]->room_flags, COLD);
	 	}		
    	strcpy(ch->p->queryprompt, "\n\rSet the room to be a DRINK room? is there water here? (y or n)> ");
    	ch->p->querycommand=1009;	
    	return;
    }
	if(cmd==1009){
		if(buf[0]=='Y'||buf[o]=='y')
			SET_BIT(world[ch->in_room]->room_flags,DRINKROOM);	
		if(buf[0]=='N'||buf[o]=='n')
			REMOVE_BIT(world[ch->in_room]->room_flags,DRINKROOM);	
    	ch->p->querycommand=0;	
    	return;
	}
 }

*/

#define NUM_ROOM_FLAGS 20

void do_editroom_roomflags(struct char_data *ch, char *argument, int cmd)
{
  char buf2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH], buf1[300];

  int counter, number, addflag=0;
  extern char *room_bits[];

  if (IS_NPC(ch))
    return;
  
  one_argument(argument, buf2);
  
   if( cmd == 9 ) {
	ch->p->queryfunc=do_editroom_roomflags;
	ch->p->querycommand=1000;
	strcpy(ch->p->queryprompt, "(# of Tag to Toggle, 0 to quit)> ");
  }

  if(cmd == 1000) {
    if(*buf2) {
	  if(is_number(buf2)) {
		number = atoi(buf2);
		if(number < 0 || number > NUM_ROOM_FLAGS) {
		   send_to_char("INVALID ROOM FLAG. \r\n",ch);
		   return;
		}
		if(number == 0) {
		   ch->p->querycommand=0;
		   return;
		}
		if(number == 13)
			return;
			
		if (IS_SET(world[ch->in_room]->room_flags, 1 << (number - 1)))
		  REMOVE_BIT(world[ch->in_room]->room_flags, 1 << (number - 1));
		else {
		  SET_BIT(world[ch->in_room]->room_flags, 1 << (number - 1));
		  addflag = 1;
		}
		if(number == 14 && addflag) 
		   add_damage_room(ch->in_room, FIRE, 120);
		else if(number == 14)
		   remove_damage_room(ch->in_room, FIRE);

		if(number == 15 && addflag)
			add_damage_room(ch->in_room, GAS, 85);
		else if(number == 15)
			remove_damage_room(ch->in_room, GAS);

		if(number == 16 && addflag)
			add_damage_room(ch->in_room, COLD, 60);
		else if(number == 16)
			remove_damage_room(ch->in_room, COLD);
	
	} /* end if is number */

   } /* end if if *buf2 */
   
 } /* end if if(cmd == 1000) */
   send_to_char("ROOM FLAGS\r\n",ch);
   for (counter = 0; counter < NUM_ROOM_FLAGS; counter += 2) {
   	sprintf(buf, "%s%2d%s) %20.20s %s%2d%s) %20.20s\r\n",
   		  GRN(ch),  counter + 1, 
                  NRM(ch), room_bits[counter],
                  GRN(ch), counter + 2, NRM(ch), 
                  counter + 1 < NUM_ROOM_FLAGS ?
                  room_bits[counter + 1] : "");
        send_to_char(buf, ch);
  }
  sprintbit((long) world[ch->in_room]->room_flags, room_bits, buf1);
  sprintf(buf, "--> %s%s\r\n\r\n%s",RED(ch), buf1, NRM(ch));
  send_to_char(buf, ch);

}



void do_editroom_exit(struct char_data *ch, char *argument, int cmd)
{
}
struct extra_descr_data *get_descr(struct char_data *ch,int number)
{
struct extra_descr_data *new_descr=NULL;
int x=0;

    if(!world[ch->in_room]->ex_description)return(NULL);
    new_descr=world[ch->in_room]->ex_description;
    while(x<number){
	if(new_descr&&new_descr->next){
	    x++;
	    new_descr=new_descr->next;
	}else{
	    return(NULL);
	}
    }
    return(new_descr);
}
void do_editroom_lookat(struct char_data *ch, char *argument, int cmd)
{
}
void do_editroom_modifiers(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH]; 
    int number;

    if (IS_NPC(ch))
	return; 
    one_argument(argument, buf); 

   if(cmd==9){
	global_color=32; 
	send_to_char("\n\r    EDIT ROOM MODIFIERS\n\r",ch); 
	global_color=0; 
	ch->p->queryfunc=do_editroom_modifiers; 
      	strcpy(ch->p->queryprompt,
	 "\n\rEnter the Movement modifier 30=takes 30 MORE movements\n\r-30 means gives 30 back cr=nochange (#/cr)> ");
	ch->p->querycommand=1000; 
	return; 
    }
    if(cmd==1000){
      if(buf[0]){
	number=atoi(buf);
	if(number<-250){
	    send_to_char("Select from -250 to 250 only.\n\r",ch);
	    return;
	}
	world[ch->in_room]->move_mod=number;
      }
      strcpy(ch->p->queryprompt,
      "\n\rEnter the pressure Modifier  cr=no change  (#/cr)> ");
      ch->p->querycommand=1001;
      return; 
    }
    if(cmd==1001){
      if(buf[0]){
	number=atoi(buf);
	if(number<-250){
	    send_to_char("Select from -250 to 250 only.\n\r",ch);
	    return;
	}
	world[ch->in_room]->pressure_mod=number;
      }
      strcpy(ch->p->queryprompt,
      "\n\rEnter the temperature Modifier  cr=no change  (#/cr)> ");
      ch->p->querycommand=1002;
      return; 
    }
    if(cmd==1002){
      if(buf[0]){
	number=atoi(buf);
	if(number<-250){
	    send_to_char("Select from -250 to 250 only.\n\r",ch);
	    return;
	}
	world[ch->in_room]->temperature_mod=number;
      }
      ch->p->querycommand=0;
      return; 
    }
}

void log_command(char *command, struct char_data *ch)
{
 if(!command || !ch)
	return;

 sprintf(log_buf,"%s at ROOM[%d] %s",GET_NAME(ch),ch->in_room,command);
 log_hd(log_buf);
 return;
}

void do_editroom(struct char_data *ch, char *argument, int cmd)
{
char buf[MAX_STRING_LENGTH];
void log_command(char *command, struct char_data *ch);

    one_argument(argument,buf);
    if(!buf[0]){
	global_color=31;
	send_to_char("\n\rSYNTAX>  EDITROOM TYPE(one of the following)\n\r",ch);
	send_to_char("------------------------------------------------------------\n\r",ch);
	global_color=36;
	send_to_char("Example  editroom n (edit the room name)\n\r",ch);
	send_to_char("Example  editroom d (edit the room description)\n\r",ch);
	send_to_char("Example  editroom f (set room flags, dark indoors etc)\n\r",ch);
	send_to_char("Example  editroom s (set room sector type, forest etc)\n\r",ch);
	send_to_char("Example  editroom r (set the restrictions, level class etc)\n\r",ch);
	send_to_char("Example  editroom m (set the modifiers, move temperature etc)\n\r",ch);
	send_to_char("Example  editroom e (edit an exit)\n\r",ch);
	send_to_char("Example  editroom l (edit things to Look at)\n\r",ch);
	send_to_char("Example  editroom a (edit the rooms Actions)\n\r",ch);
	global_color=0;
	return;
    }
    if(ch->specials.editzone>=0)
    if(ch->specials.editzone!=world[ch->in_room]->zone&&GET_LEVEL(ch)<35){
	sprintf(log_buf,"-=<(%s)>=-",GET_NAME(ch));
	if(strcmp(world[ch->in_room]->name,log_buf)){
	    send_to_char("You are not authorized to do that for this zone.\n\r",ch);
	    return;
	}
    }

    switch(buf[0]){
	case 'n':
	    if(ch->specials.editzone>=0)
	    if(ch->specials.editzone!=world[ch->in_room]->zone&&GET_LEVEL(ch)<35){
		sprintf(log_buf,"-=<(%s)>=-",GET_NAME(ch));
		if(!strcmp(world[ch->in_room]->name,log_buf)){
	    	    send_to_char("You are not authorized to do that for this zone.\n\r",ch);
		    return;
		}
    	    }
	    if(GET_LEVEL(ch)==32) 
		log_command("edits the room NAME",ch);

	    ch->desc->str = &world[ch->in_room]->name; 
	    ch->desc->max_str = 79; 
	    strcpy(ch->desc->editing,"Room name");
	    ch->desc->oneline=TRUE;
	    break;
	case 'd':
	    global_color=31;
	    act("$n starts editing the room description",TRUE,ch,0,0,TO_ROOM);
	    global_color=0;
	    if(GET_LEVEL(ch)==32)
		log_command("edits the room DESCRIPTION.",ch);

	    /* juice -- save the old room desc for later*/
	    ch->player.short_descr = my_free(ch->player.short_descr);
	    ch->player.short_descr = str_dup(world[ch->in_room]->description);
	    /* end changes */
	    ch->desc->str = &world[ch->in_room]->description; 
	    ch->desc->max_str = 1000; 
	    strcpy(ch->desc->editing,"Room Description");
	    break;
	case 'f':
	    if(GET_LEVEL(ch)==32)
		log_command("edits the ROOMFLAGS.",ch);
	    do_editroom_roomflags(ch,"",9);
	    break;
	case 's':
	    if(GET_LEVEL(ch)==32)
		log_command("edits the SECTOR type.",ch);
	    do_editroom_sectortype(ch,"",9);
	    break;
	case 'r':
	    if(GET_LEVEL(ch)==32)
		log_command("edits the room RESTRICTIONS.",ch);

	    do_editroom_restrictions(ch,"",9);
	    break;
	case 'm':
	    if(GET_LEVEL(ch)==32) {
		send_to_char("You are not authorized to edit room modifiers.\r\n",ch);
		return;
	    }
	    do_editroom_modifiers(ch,"",9);
	    break;
	case 'e':
	    if(GET_LEVEL(ch)==32) 
		log_command("edits room EXITS\r\n",ch);
	    
	    do_editroom_exit(ch,"",9);
	    break;
	case 'l':
	    if(GET_LEVEL(ch)==32) 
		log_command("edits the room LOOKATS.",ch);
	   
	    do_editroom_lookat(ch,"",9);
	    break;
	case 'a':
	    if(GET_LEVEL(ch)==32) {
		send_to_char("You are not authorized.\r\n",ch);
		return;
	    }
	    do_editroom_actions(ch,"",9);
	    break;
	default:
	    send_to_char("Option not recognized.\n\r",ch);
	    break;
    }

}

void do_afk(struct char_data *ch, char *argument, int cmd)
{

    if(!ch->specials.afk){
       ch->specials.afk=1;
       send_to_char("AWAY FROM KEYBOARD MODE NOW [ON]!\n\r",ch);
       act( "$n Just went into AFK MODE", TRUE, ch, 0, 0, TO_ROOM );
       if(argument[0])
	   strcpy(&ch->specials.afk_text[0],argument);
    }else{
       ch->specials.afk=0;
       send_to_char("AWAY FROM KEYBOARD MODE NOW [OFF]!\n\r",ch);
       act( "$n is BACK from AFK mode.", TRUE, ch, 0, 0, TO_ROOM );
       ch->specials.afk_text[0]=0;
    }
}

void do_gohome(struct char_data *ch, char *argument, int cmd)
{
int location;
	if(!ch->specials.home_number){
	   send_to_char("You Don't OWN a HOME!\n\r",ch);
	   return;
	}
	if((!world[ch->specials.home_number])||
	   ((!IS_SET(world[ch->specials.home_number]->room_flags, HOME))&&
	    (GET_LEVEL(ch) < 32))){
	    send_to_char("ILLEGAL room number! Resetting\n\r",ch);
		sprintf(log_buf,"## %s has a bad home number %d.  Resetting to none.",
			GET_NAME(ch),ch->specials.home_number);
		log_hd(log_buf);
		ch->specials.home_number = 0;
	    return;
	}
	if(GET_LEVEL(ch)<33){
/*
	if(world[ch->in_room]->zone==65){
	    send_to_char("Doesn't seem to work from the ship.\n\r",ch);
	    return;
	}
*/
	if(world[ch->specials.home_number]->zone!=world[ch->in_room]->zone){
	    send_to_char("Your home is too far away.\n\r",ch);
	    return;
 	}
	}
		global_color=33;
		send_to_char("Welcome Back!\n\r",ch);
		global_color=0;
	if(ch->specials.fighting){
	   send_to_char("You're FIGHTING!\n\r",ch);
	   return;
	}
               location = ch->specials.home_number;
	       if(!world[location]){
		  send_to_char("Please tell Vryce your home_number is invalid\n\r",ch);
		  return;
	       }	
/*
	       if(GET_LEVEL(ch)<33){
	       if(zone_table[world[location]->zone].continent!=GET_CONTINENT(ch)){
		   send_to_char("The magic of gohome doesn't work across the ocean.\n\r",ch);
		   return;
	       }
	       if(world[ch->in_room]->zone==65){
		   send_to_char("No gohomes from the ship.\n\r",ch);
		   return;
	       }
	       }
*/
	       global_color=31;
	       act( "$n disappears to $s home.", TRUE, ch, 0, 0, TO_ROOM );
	       char_from_room( ch );
	       char_to_room( ch, location);
	       act( "$n appears in the middle of the room.", TRUE, ch, 0, 0, TO_ROOM);
	       send_to_char("PHHHHHWWWTT! POOF! You're Home!\n\r",ch);
	       global_color=0;
	       do_look( ch, "", 0 );
	       return;
}

void do_color(struct char_data *ch, char *argument, int cmd)
{
    if(ch->specials.ansi_color!=69){
	ch->specials.ansi_color=69;
        send_to_char("\033[0m\033[2J",ch);
	global_color=31;
	send_to_char("ANSI COLOR Enabled.\n\r",ch);
	global_color=0;
	return;
    }
    send_to_char("ANSI COLOR Disabled.\n\r",ch);
    ch->specials.ansi_color=0;
    
}
void do_ll_set(struct char_data *ch, char *argument, int cmd)
{
    if(!ch->specials.ll_set){
	send_to_char("Displaying of Auto Room Stats Enabled.\n\r",ch);
	ch->specials.ll_set=1;
	return;
    }
    send_to_char("Displaying of Auto Room Stats Disabled.\n\r",ch);
    ch->specials.ll_set=0;
    
}
void do_autoexit(struct char_data *ch, char *argument, int cmd)
{
    if(!ORIGINAL(ch)->specials.autoexit){
	send_to_char("Displaying EXITS automatically Disabled.\n\r",ch);
	ORIGINAL(ch)->specials.autoexit=1;
	return;
    }
    send_to_char("Displaying EXITS automatically Enabled.\n\r",ch);
    ORIGINAL(ch)->specials.autoexit=0;
    
}
void do_display_autosave(struct char_data *ch, char *argument, int cmd)
{
    if(ch->specials.display_autosave){
	send_to_char("Displaying when AUTOSAVE'd disabled.\n\r",ch);
	ch->specials.display_autosave=0;
	return;
    }
    send_to_char("Displaying when AUTOSAVE'd enabled.\n\r",ch);
    ch->specials.display_autosave=1;
    
}

bool if_allowed_to_attack(struct char_data *ch, struct char_data *vict)
{

#ifdef PACIFIST
	send_to_char("You feel so peaceful you lay down your weapon and hug your enemy.\n\r",ch);
	return(FALSE);
#else
    if(IS_UNDEAD(ch)
	&& (vict->nr == 9801)
	&& (vict->specials.fighting)
	)
	{
	send_to_char("This Necromancer has other problems right now.\n\r",ch);
	return(FALSE);
	}

    if((ch->nr == 9801) && (IS_UNDEAD(ch->specials.fighting)))
	{
	send_to_char("You decide to let the corpse fight for its own spirit.",ch);
	return(FALSE);
	}

    if(ch->specials.fighting == vict)
	return(TRUE);

    if(GET_LEVEL(ch) > 32) return(TRUE);

    if(IS_SET(world[ch->in_room]->room_flags,NEUTRAL)
	&& (!IS_NPC(ch))
	&& (!IS_NPC(vict))
	&& (vict->specials.fighting)
	&& (IS_NPC(vict->specials.fighting))
	)
	{
	send_to_char("Hey! You're trying to turn NPK into CPK!???!",ch);
	return(FALSE);
	}

    if(IS_DEAD(vict) || ((vict->nr == 9800)&&(vict->specials.death_timer > 1)))
	{
	send_to_char("You want to kill someone who's already dead?\n\r",ch);
	return(FALSE);
	}

    if(in_a_shop(ch)){
        send_to_char(
			"You realize it is not polite to fight in a public shop.\n\r",ch);
        return(FALSE);
    }
	if(IS_SET(world[ch->in_room]->room_flags,LAWFULL)
		&& IS_NPC(vict) && IS_AFFECTED( vict, AFF_CHARM )
		&& vict->master
		&& (vict->in_room == vict->master->in_room)
		)
		{
		send_to_char("You wouldn't attack someone's pet, would you?\n\r",ch);
		return(FALSE);
		}

    if(IS_NPC(vict))return(TRUE);
    if(vict==ch)return(TRUE);
    if(!vict->specials.fighting&&!vict->desc){
	global_color=33;
	send_to_char("You cannot start fighting someone who lost link!\n\r",ch);
	global_color=0;
	return(FALSE);
    }
    if(!IS_SET(world[ch->in_room]->room_flags,NEUTRAL)&&
	!IS_SET(world[ch->in_room]->room_flags,CHAOTIC)){
        send_to_char("You are only allowed to ATTACK players in NEUTRAL or CHAOTIC Areas!\n\r",ch); 
       return(FALSE);
    }

    return(TRUE);
#endif
}
char *get_title(struct char_data *ch)
{
    if(ch->player.title){

/*	if(!ch->desc){
	   sprintf(log_buf," [*LOST LINK*] %s",ch->player.title);
	   return(log_buf);
	}
*/
	if(ch->desc&&ch->desc->str){
	   sprintf(log_buf," [*EDITING (%s) PLEASE BE QUIET*] ",ch->desc->editing);
	   return(log_buf);
	}
/*	if(ch->specials.afk){
	   sprintf(log_buf," [*AFK*] %s",ch->player.title);
	   return(log_buf);
	}
*/
	return(ch->player.title);
    }else
	return("NOTITLE");
}
void do_unalias(struct char_data *ch, char *argument, int cmd)
{
char buf[MAX_INPUT_LENGTH];
char alias[MAX_INPUT_LENGTH];
int i=0,a=0;
char flag;
	if(IS_NPC(ch))return;
	one_argument(argument,buf);
	if(!*buf){
	    send_to_char("Yes but UNALIAS What?\n\r",ch);
	    send_to_char("SYNTAX  unalias alias\n\r",ch);
	    return;
	}
	flag=0;
	for(i=0;i<5;i++){
	    one_argument(&ch->p->alias[i][0],alias);
	    if(!strcmp(alias,buf)){
		flag=1;
		sprintf(log_buf,"Alias (%s) removed.\n\r",buf);
		send_to_char(log_buf,ch);
		for(a=i;a<4;a++)
		    strcpy(&ch->p->alias[a][0],&ch->p->alias[a+1][0]);
		ch->p->alias[4][0]=MED_NULL;
	    }
	}
	if(!flag){
	   sprintf(log_buf,"alias (%s) is not defined!\n\r",buf);
	   send_to_char(log_buf,ch);
	}
	
}
void do_alias(struct char_data *ch, char *argument, int cmd)
{
char buf[MAX_INPUT_LENGTH];
char alias[MAX_INPUT_LENGTH];
char text[MAX_INPUT_LENGTH];
char command[MAX_INPUT_LENGTH];
int i=0,a=0;
    if(IS_NPC(ch))return;
    while((ch->p->alias[i][0])&&(i<5))i++;  /* get next blank */

    if(ch->p->querycommand==300){
	if(argument[0]=='Y'||argument[0]=='y'){
    	    ch->p->queryfunc=do_alias;
	    strcpy(ch->p->queryprompt,"Type in alias [syntax example >av assist Vryce] > ");
    	    ch->p->querycommand=301;
        }else
	    ch->p->querycommand=0;
        return;
    }
    if(ch->p->querycommand==301){
	if(argument[0]){
	    if(strlen(argument)>80)
		send_to_char("Alias is too long. Limit it to 80 characters.\n\r", ch);
	    else{
		set_to_lower(text,argument);
	        half_chop(argument,alias,command);
		if(!command[0])
		    send_to_char("Yes, but where's the command?\n\r", ch);
		else{
		    if(!str_cmp(alias,"gre")||!str_cmp(alias,"grep")){
			send_to_char("You cannot use an alias to do the GREP command.\n\r",ch);
			return;
		    }
            if(strstr(text,"moogooboozoo")){
                send_to_char("Huh?\n\r",ch);
                return;
            }
			if(strstr(text,"~")){
				send_to_char("Huh?\n\r", ch);
				return;
			}
		    if(strstr(text,"gre ")||strstr(text,"grep ")){
			send_to_char("You cannot use an alias to do the GREP command.\n\r",ch);
			return;
		    }
		    strcpy(&ch->p->alias[i][0],argument);
 		    sprintf(buf,"OK, done! [%s] will now do this [%s].\n\r",alias,command);
		    send_to_char(buf,ch);
		}
	    }
        }
	ch->p->querycommand=0;
        return;
    }

    if(i>=5){
	send_to_char("[*YOU ARE AT YOUR LIMIT OF 5 ALIASES!*]\n\r",ch);
        send_to_char("Your defined aliases are:\n\r", ch);
	for(a=0;a<i;a++){
	    send_to_char(&ch->p->alias[a][0], ch);
	    send_to_char("\n\r", ch);
	}
	return;
    }
    send_to_char("Your defined aliases are:\n\r", ch);
    if(i==0)
	send_to_char("There are currently no aliases, you can have five.\n\r", ch);
    else
	for(a=0;a<i;a++){
	    send_to_char(&ch->p->alias[a][0], ch);
	    send_to_char("\n\r", ch);
	}
    ch->p->queryfunc=do_alias;
    if(i)
	strcpy(ch->p->queryprompt,"Do you want to create another alias? (y/n)> ");
    else
	strcpy(ch->p->queryprompt,"Do you want to create an alias? (y/n)> ");
    ch->p->querycommand=300;
    return;
	
}

void do_donate(struct char_data *ch, char *argument, int cmd)
{
char buf[MAX_INPUT_LENGTH];
char text[MAX_INPUT_LENGTH];
struct obj_data *obj=NULL;
struct char_data *dude=NULL;
int room;

    room=3;
    if(GET_CONTINENT(ch)==TRELLOR)
        room=4553;
    one_argument(argument, buf);
    if(!buf){
	send_to_char("Yes, but donate what?\n\r", ch);
	return;
    }
    if(!(obj=get_obj_in_list_vis(ch, buf, ch->carrying))){
	send_to_char("Your intentions are admirable, but, you don't have that!\n\r", ch);
	return;
    }
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)){
	if(obj->item_number==10||obj->item_number==16)
	    send_to_char("You would never dream of donating such a precious gift.\n\r", ch);
	else
	    send_to_char("You can't let go! It must be cursed!\n\r", ch);
	return;
    }
    obj_from_char(obj);
    obj_to_room(obj, room);
    sprintf(text,"You send %s to the donation room.\n\r", obj->short_description);    
    send_to_char(text,ch);
    sprintf(text,"$n sends %s to the donation room.\n\r", obj->short_description);    
    act(text,TRUE, ch, 0, 0, TO_ROOM);
    for(dude=world[room]->people; dude; dude=dude->next_in_room)
	send_to_char("[BANG!] A crack in space opens up and something flies through!\n\r[PHHFFfffft] The crack shrinks to a pinpoint and disappears.\n\r", dude);
}
void do_godsend(struct char_data *ch, char *argument, int cmd)
{
char buf[MAX_INPUT_LENGTH];
struct obj_data *obj=NULL;
int x=0;

    one_argument(argument, buf);
    if(!buf){
	send_to_char("Yes, but send what to all gods?\n\r", ch);
	return;
    }
    if(!(obj=get_obj_in_list_vis(ch, buf, ch->carrying))){
	send_to_char("Your intentions are admirable, but, you don't have that!\n\r", ch);
	return;
    }
    while(god_list[x][0]!='$'){
       sprintf(log_buf,"%s %s",buf,god_list[x++]);
       post_office(ch,999,log_buf);
    }
}
void do_noemote(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict=NULL;
    struct obj_data *dummy=NULL;
    char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
	return;

    one_argument(argument, buf);

    if (!*buf)
	send_to_char("Noemote who?\n\t", ch);

    else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
	send_to_char("Couldn't find any such creature.\n\r", ch);
    else if (!vict->desc)
	send_to_char("Can't do that to a beast.\n\r", ch);
    else if (GET_LEVEL(ORIGINAL(vict)) >= GET_LEVEL(ch))
	act("$E might object to that ... better not.",
	0, ch, 0, vict, TO_CHAR);
    else if (IS_SET(ORIGINAL(vict)->specials.act, PLR_NOEMOTE))
    {
	send_to_char("You can emote again.\n\r", vict);
	send_to_char("NOEMOTE removed.\n\r", ch);
	REMOVE_BIT(ORIGINAL(vict)->specials.act, PLR_NOEMOTE);
    }
    else
    {
	send_to_char("The gods take away your ability to emote!\n\r", vict);
	send_to_char("NOEMOTE set.\n\r", ch);
	SET_BIT(ORIGINAL(vict)->specials.act, PLR_NOEMOTE);
    }
}


void do_notell(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict=NULL;
    struct obj_data *dummy=NULL;
    char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
	return;

    one_argument(argument, buf);

    if (!*buf)
	if (IS_SET(ORIGINAL(ch)->specials.act, PLR_NOTELL))
	{
	    send_to_char("You can now hear tells again.\n\r", ch);
	    REMOVE_BIT(ORIGINAL(ch)->specials.act, PLR_NOTELL);
	}
	else
	{
	    send_to_char("From now on, you can't use tell.\n\r", ch);
	    SET_BIT(ORIGINAL(ch)->specials.act, PLR_NOTELL);
	}
    else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
	send_to_char("Couldn't find any such creature.\n\r", ch);
    else if (!vict->desc)
	send_to_char("Can't do that to a beast.\n\r", ch);
    else if (GET_LEVEL(ORIGINAL(vict)) >= GET_LEVEL(ch))
	act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    else if (IS_SET(ORIGINAL(vict)->specials.act, PLR_NOTELL))
    {
	send_to_char("You can use telepatic communication again.\n\r", vict);
	send_to_char("NOTELL removed.\n\r", ch);
	REMOVE_BIT(ORIGINAL(vict)->specials.act, PLR_NOTELL);
    }
    else
    {
	send_to_char(
	"The gods take away your ability to use telepatic communication!\n\r",
	    vict);
	send_to_char("NOTELL set.\n\r", ch);
	SET_BIT(ORIGINAL(vict)->specials.act, PLR_NOTELL);
    }
}


void do_freeze(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict=NULL;
    struct obj_data *dummy=NULL;
    char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
	return;

    one_argument(argument, buf);

    if (!*buf)
	send_to_char("Freeze who?\n\r", ch);

    else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
	send_to_char("Couldn't find any such creature.\n\r", ch);
    else if (!vict->desc)
	send_to_char("Can't do that to a beast.\n\r", ch);
    else if (GET_LEVEL(ORIGINAL(vict)) >= GET_LEVEL(ch))
	act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    else if (IS_SET(ORIGINAL(vict)->specials.act, PLR_FREEZE))
    {
	send_to_char("You now can do things again.\n\r", vict);
	send_to_char("FREEZE removed.\n\r", ch);
	REMOVE_BIT(ORIGINAL(vict)->specials.act, PLR_FREEZE);
    }
    else
    {
	send_to_char("The gods take away your ability to ...\n\r", vict);
	send_to_char("FREEZE set.\n\r", ch);
	SET_BIT(ORIGINAL(vict)->specials.act, PLR_FREEZE);
    }
}


void do_log(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict=NULL;
    struct obj_data *dummy=NULL;
    char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
	return;

    one_argument(argument, buf);

    if (!*buf)
		send_to_char("Log who?\n\r", ch);
    else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
		send_to_char("Couldn't find any such creature.\n\r", ch);
    else if (!vict->desc)
		send_to_char("Can't do that to a beast.\n\r", ch);
    else if (GET_LEVEL(vict) > GET_LEVEL(ch)||IS_PLAYER(vict,"Vryce"))
		act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    else if (IS_SET(ORIGINAL(vict)->specials.act, PLR_LOG))
    {
		send_to_char("LOG removed.\n\r", ch);
		REMOVE_BIT(ORIGINAL(vict)->specials.act, PLR_LOG);
    }else{
		send_to_char("LOG set.\n\r", ch);
		SET_BIT(ORIGINAL(vict)->specials.act, PLR_LOG);
    }
}

void do_templog(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict=NULL;
    struct obj_data *dummy=NULL;
    char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
	return;

    one_argument(argument, buf);

    if (!*buf)
		send_to_char("Log who?\n\r", ch);
    else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
		send_to_char("Couldn't find any such creature.\n\r", ch);
    else if (!vict->desc)
		send_to_char("Can't do that to a beast.\n\r", ch);
    else if (GET_LEVEL(vict) > GET_LEVEL(ch)||IS_PLAYER(vict,"Vryce"))
		act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
	else if(strcmp(GET_NAME(ch),"Vryce"))
		send_to_char("What?!?",ch);
    else if (vict->desc->templog)
    {
		send_to_char("LOG removed.\n\r", ch);
		vict->desc->templog=0;
    }else{
		send_to_char("LOG set.\n\r", ch);
		vict->desc->templog=1;
    }
}

void do_wizlock(struct char_data *ch, char *argument, int cmd)
{
    int level=0;
    char buf[MAX_INPUT_LENGTH];

    if(IS_NPC(ch))return;

    if ( wizlock ) {
	wizlock = FALSE;
	sprintf(log_buf,"Game has been un-wizlocked by %s.",GET_NAME(ch));
	log_hd(log_buf);
	send_to_char("Game unwizlocked.\n\r", ch);
    } else {
	one_argument(argument,buf);
	if(!*buf)
		level = 31;
	else if(!is_number(buf)) 
		level = 31;
	else level = atoi(buf);

	if(level > 35)
		level = 35;
	if(level < 1)
		level = 1;

	wizlock = level;

	sprintf(log_buf,"Game has been un-wizlocked by %s at %d.",GET_NAME(ch),wizlock);
	log_hd(log_buf);
	sprintf(log_buf,"Game Wizlocked at %d.\r\n",wizlock);
	send_to_char(log_buf, ch);
    }
}




/* This routine is used by 34+ level ONLY to set 
   specific char/npc variables, including skills */

void do_set(struct char_data *ch, char *argument, int cmd)
{
    char *values[] = {
	"age","sex","class","level","height","weight","str","stradd",
	"int","wis","dex","con","gold","exp","mana","hit","move",
	"sessions","alignment","thirst","drunk","full","pk","eggs","sta","\n"
    };
    struct char_data *vict=NULL;
    char name[100], buf2[100], buf[100], help[MAX_STRING_LENGTH];
    int skill, value, i;

    if(IS_NPC(ch))return;
    argument = one_argument(argument, name);
    if (!*name) /* no arguments. print an informative text */
    {
	send_to_char(
	    "NEW SYNTAX:\n\rset <name> <field> <value>\n\r", ch);

	strcpy(help, "\n\rField being one of the following:\n\r");
	for (i = 1; *values[i] != '\n'; i++)
	{
	    sprintf(help + strlen(help), "%18s", values[i]);
	    if (!(i % 4))
	    {
		strcat(help, "\n\r");
		send_to_char(help, ch);
		*help = '\0';
	    }
	}
	if (*help)
	    send_to_char(help, ch);
	send_to_char("\n\r", ch);
	return;
    }
    if (!(vict = get_char_vis(ch, name)))
    {
	send_to_char("No living thing by that name.\n\r", ch);
	return;
    }
	argument = one_argument(argument,buf);
	if (!*buf)
	{
	    send_to_char("Field name expected.\n\r", ch);
	    return;
	}
	if ((skill = old_search_block(buf, 0, strlen(buf), values, 1)) < 0)
	{
	    send_to_char(
		"No such field is known. Try 'set' for list.\n\r", ch);
	    return;
	}
	skill--;
	argument = one_argument(argument,buf);
	if (!*buf)
	{
	    send_to_char("Value for field expected.\n\r", ch);
	    return;
	}
	sprintf(buf2,
		"%s sets %s's %s to %s.",
		GET_NAME(ch),GET_NAME(vict),values[skill],buf);
	switch (skill) {
	    case 0: /* age */
	    {
		value = atoi(buf);
		if ((value < 16) || (value > 79))
		{
		    send_to_char("Age must be between 17 and 80.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		value = value - 17; /* code adds 17 to age */
		/* set age of victim */
		vict->player.time.birth = 
		    time(0) - (long)value*(long)SECS_PER_MUD_YEAR;
	    };
	    break;
	    case 1: /* sex */
	    {
		if (str_cmp(buf,"m") && str_cmp(buf,"f") && str_cmp(buf,"n"))
		{
		    send_to_char("Sex must be 'm','f' or 'n'.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set sex of victim */
		switch(*buf) {
		    case 'm':vict->player.sex = SEX_MALE;   break;
		    case 'f':vict->player.sex = SEX_FEMALE; break;
		    case 'n':vict->player.sex = SEX_NEUTRAL;break;
		}
	    }
	    break;
	    case 2: /* class */
	    {
/* juice
 * If people are unhappy with their class, they should start a new char.
 * Nobody has any business multiclassing their friends.
 */
		if (str_cmp(buf,"m") && str_cmp(buf,"c") && 
		    str_cmp(buf,"w") && str_cmp(buf,"t"))
		{
		    send_to_char("Class must be 'm','c','w' or 't'.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set class of victim */
		switch(*buf) {
		    case 'm':vict->player.class = CLASS_MAGIC_USER; break;
		    case 'c':vict->player.class = CLASS_CLERIC;     break;
		    case 'w':vict->player.class = CLASS_WARRIOR;    break;
		    case 't':vict->player.class = CLASS_THIEF;      break;
		}
	    }
	    break;
	    case 3: /* level */
	    {
		value = atoi(buf);
		if ((((value < 0) || (value > 31))&&!IS_NPC(vict))||
                   ((value < 0) || (value > 127)))
		{
		    send_to_char(
			"Level must be between 0 and 31.\n\r", ch );
		    return;
		}
		log_hd(buf2);
		/* set level of victim */
		vict->player.level = value;
	    }
	    break;
	    case 4: /* height */
	    {
		value = atoi(buf);
		if ((value < 100) || (value > 250))
		{
		    send_to_char("Height must be more than 100 cm\n\r", ch);
		    send_to_char("and less than 251 cm.\n\r", ch); 
		    return;
		}
		log_hd(buf2);
		/* set height of victim */
		vict->player.height = value;
	    }       
	    break;
	    case 5: /* weight */
	    {
		value = atoi(buf);
		if ((value < 100) || (value > 250))
		{
		    send_to_char("Weight must be more than 100 stones\n\r", ch);
		    send_to_char("and less than 251 stones.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set weight of victim */
		vict->player.weight = value;
	    }
	    break;
	    case 6: /* str */
	    {
		value = atoi(buf);
		if ((value <= 0) || (value > 18))
		{
		    send_to_char("Strength must be more than 0\n\r", ch);
		    send_to_char("and less than 19.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set original strength of victim */
		vict->tmpabilities.str = value;
		vict->abilities.str = value;
	    }
	    break;
	    case 7: /* stradd */
	    {
		send_to_char( "Strength addition not supported.\n\r", ch );
	    }
	    break;
	    case 8: /* int */
	    {
		value = atoi(buf);
		if ((value <= 0) || (value > 18))
		{
		    send_to_char("Intelligence must be more than 0\n\r", ch);
		    send_to_char("and less than 19.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set original INT of victim */
		vict->tmpabilities.intel = value;
		vict->abilities.intel = value;
	    }
	    break;
	    case 9: /* wis */
	    {
		value = atoi(buf);
		if ((value <= 0) || (value > 18))
		{
		    send_to_char("Wisdom must be more than 0\n\r", ch);
		    send_to_char("and less than 19.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set original WIS of victim */
		vict->tmpabilities.wis = value;
		vict->abilities.wis = value;
	    }
	    break;
	    case 10: /* dex */
	    {
		value = atoi(buf);
		if ((value <= 0) || (value > 18))
		{
		    send_to_char("Dexterity must be more than 0\n\r", ch);
		    send_to_char("and less than 19.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set original DEX of victim */
		vict->tmpabilities.dex = value;
		vict->abilities.dex = value;
	    }
	    break;
	    case 11: /* con */
	    {
		value = atoi(buf);
		if ((value <= 0) || (value > 18))
		{
		    send_to_char("Constitution must be more than 0\n\r", ch);
		    send_to_char("and less than 19.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set original CON of victim */
		vict->tmpabilities.con = value;
		vict->abilities.con = value;
	    }
	    break;
	    case 12: /* gold */
	    {
		value = atoi(buf);
		log_hd(buf2);
		/* set original gold of victim */
		vict->points.gold = value;
	    }
	    break;
	    case 13: /* exp */
	    {
		value = atoi(buf);
		if ((value <= 0) || (value > 1400000000))
		{
		    send_to_char(
			"Experience-points must be more than 0\n\r", ch);
		    send_to_char("and less than 1400000001.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set original exp of victim */
		vict->points.exp = value;
	    }
	    break;
	    case 14: /* mana */
	    {
		value = atoi(buf);
		if ((value <= -100) || (value > 9994))
		{
		    send_to_char("Mana-points must be more than -100\n\r", ch);
		    send_to_char("and less than 9995.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set original mana of victim */
		vict->points.max_mana = value;
	    }
	    break;
	    case 15: /* hit */
	    {
		value = atoi(buf);
		if ((value <= -10) || (value > 30000))
		{
		    send_to_char("Hit-points must be more than -10\n\r", ch);
		    send_to_char("and less than 30001.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set original hit of victim */
		vict->points.max_hit = value;
	    }
	    break;
	    case 16: /* move */
	    {
		value = atoi(buf);
		if ((value <= -100) || (value > 9917))
		{
		    send_to_char("Move-points must be more than -100\n\r", ch);
		    send_to_char("and less than 9918.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set original move of victim */
		vict->points.max_move = value;
	    }
	    break;
	    case 17: /* sessions */
	    {
		value = atoi(buf);
		if ((value < 0) || (value > 100))
		{
		    send_to_char("Sessions must be more than 0\n\r", ch);
		    send_to_char("and less than 101.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set original sessions of victim */
		vict->specials.practices = value;
	    }
	    break;
	    case 18: /* alignment */
	    {
		value = atoi(buf);
		if ((value < -1000) || (value > 1000))
		{
		    send_to_char("Alignment must be more than -1000\n\r", ch);
		    send_to_char("and less than 1000.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set original alignment of victim */
		vict->specials.alignment = value;
	    }
	    break;
	    case 19: /* thirst */
	    {
		value = atoi(buf);
		if ((value < -1) || (value > 100))
		{
		    send_to_char("Thirst must be more than -2\n\r", ch);
		    send_to_char("and less than 101.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set original thirst of victim */
		vict->specials.conditions[THIRST] = value;
	    }
	    break;
	    case 20: /* drunk */
	    {
		value = atoi(buf);
		if ((value < -1) || (value > 100))
		{
		    send_to_char("Drunk must be more than -2\n\r", ch);
		    send_to_char("and less than 101.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set original drunk of victim */
		vict->specials.conditions[DRUNK] = value;
	    }
	    break;
	    case 21: /* full */
	    {
		value = atoi(buf);
		if ((value < -1) || (value > 100))
		{
		    send_to_char("Full must be more than -2\n\r", ch);
		    send_to_char("and less than 101.\n\r", ch);
		    return;
		}
		log_hd(buf2);
		/* set original full of victim */
		vict->specials.conditions[FULL] = value;
	    }
	    break;
            case 22: /* PKs */
            {
                value = atoi(buf);
                log_hd(buf2);
                /* set victim's player kills */
                vict->specials.numpkills = value;
            }
	    break;
            case 23: /* Eggs */
            {
                value = atoi(buf);
                log_hd(buf2);
                /* set victim's eggs */
                vict->specials.eggs = value;
            }
	    break;
	    case 24: /* sta */
	    {
		value = atoi(buf);
		if ((value <= 0) || (value > 18))
		{
			if(!IS_NPC(vict)){
			    send_to_char("Stamina must be more than 0\n\r", ch);
			    send_to_char("and less than 19.\n\r", ch);
			    return;
			}
		}
		log_hd(buf2);
		/* set original STA of victim */
		vict->tmpabilities.sta = value;
		vict->abilities.sta = value;
	    }
	    break;
	}
    send_to_char("Ok.\n\r", ch);
}

void do_imm(struct char_data *ch, char *argument, int cmd)
{
	if (IS_NPC(ch))
		return;

	if (get_total_level(ch) < 124 && GET_LEVEL(ch) < 32) {
		send_to_char("You must be level 124 to use the immortal channel.\n", ch);
		return;
	}

	for (; *argument == ' '; argument++);

	if (!(*argument)) {
		send_to_char("What do you want to tell all immortals?\n", ch);
		return;
	}

	char buf[MAX_INPUT_LENGTH] = { 0 };
	sprintf(buf, "%s:: %s\n", GET_NAME(ch), argument);

	global_color = 36;
	for (struct descriptor_data* i = descriptor_list; i; i = i->next) {
		if ((!i->connected || i->connected == CON_SOCIAL_ZONE) && (GET_LEVEL(i->character) > 31 || get_total_level(i->character) == 124)) {
			send_to_char(buf, i->character);
		}
	}
	global_color = 0;
}

void do_wiz(struct char_data *ch, char *argument, int cmd)
{
	return;

    char buf1[MAX_STRING_LENGTH];
    struct descriptor_data *i=NULL;
	int mylevel=31;
	char mybuf[MAX_INPUT_LENGTH];

    if(IS_NPC(ch))return;
    for (; *argument == ' '; argument++);

    if (!(*argument))
	send_to_char(
	    "What do you want to tell all gods and immortals?\n\r", ch);
    else
    {
	if(argument[0]=='#'){
		argument++;
		one_argument(argument,mybuf);
		if(is_number(mybuf)){
			half_chop(argument, mybuf, argument);
			mylevel=MIN(atoi(mybuf), 35);
			if(mylevel<31){
				send_to_char("Thats not a god level.\n", ch);
				return;
			}
			if(mylevel>GET_LEVEL(ch)){
				send_to_char("You can't speak at that level.\n",ch);
				return;
			}
		}
	}	
	if(GET_LEVEL(ch)>=31)global_color=36;
	else global_color=34;
	if(cmd==5)
		sprintf(buf1, "%s\r\n", argument);
	else
		if(mylevel>31)
			sprintf(buf1, "%s [%d]: %s\r\n", GET_NAME(ch), mylevel, argument);
		else
			sprintf(buf1, "%s: %s\r\n", GET_NAME(ch), argument);
/*	act(buf1, FALSE, ch, 0, 0, TO_CHAR);*/

	for (i = descriptor_list; i; i = i->next)
	if (/*i->character != ch &&*/ (!i->connected || i->connected == CON_SOCIAL_ZONE) && 
		GET_LEVEL(i->character) >= mylevel)
	    if(cmd==5){
		if(IS_SET(i->character->specials.god_display,GOD_INOUT))
			send_to_char(buf1, i->character);
/*			act(buf1, FALSE, ch, 0, i->character, TO_VICT);*/
	    }else
	        if(IS_SET(i->character->specials.god_display,GOD_ONOFF))
			send_to_char(buf1, i->character);
/*		    	act(buf1, FALSE, ch, 0, i->character, TO_VICT);*/
	global_color=0;
    }
}

void do_tap(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *obj=NULL;
    char name[100];
    char buf[MAX_INPUT_LENGTH];
    int xp;

    if( IS_NPC(ch) )
	{
	send_to_char("The gods don't accept sacrifices from monsters.\n\r",ch);
	return;
	}

    one_argument (argument, name);

    if (!*name || !str_cmp(name, GET_NAME(ch)) )
    {
	act( "$n offers $mself to $s god, who graciously declines.",
	    FALSE, ch, 0, 0, TO_ROOM);
	act( "Your god appreciates your offer and may accept it later.",
	    FALSE, ch, 0, 0, TO_CHAR);
	return;
    }

    obj = get_obj_in_list_vis( ch, name, world[ch->in_room]->contents );
    if ( obj == NULL )
    {
	act( "You can't find that object.",
	    FALSE, ch, 0, 0, TO_CHAR);
	return;
    }

    if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
	act( "$p is not an acceptable sacrifice.",
	    FALSE, ch, obj, 0, TO_CHAR );
	return;
    }

    if ( (ch->in_room == 3) || (ch->in_room == 4553) )
    {
        act( "$n sacrifices $p to $s god.", FALSE, ch, obj, 0, TO_ROOM);
        act( "You tidy up the donation room.", FALSE, ch, obj, 0, TO_CHAR
);
        extract_obj(obj);
        return;
    }

    if ( GET_ITEM_TYPE(obj) != ITEM_CONTAINER || obj->obj_flags.value[3] != 1 )
    {
	act( "$n sacrifices $p to $s god.", FALSE, ch, obj, 0, TO_ROOM );
	sprintf(name,"You get %d gold coins for your sacrifice",
		(int)(obj->obj_flags.cost-(int)(obj->obj_flags.cost*.93)));
	act( name,FALSE, ch, obj, 0, TO_CHAR );
	GET_GOLD(ch) += (int)(obj->obj_flags.cost-(int)(obj->obj_flags.cost*.93));
	extract_obj(obj);
    }
    else if ( obj->obj_flags.cost_per_day != 100000 )
    {
	act( "Such a sacrifice would be very unwise.",
	    FALSE, ch, obj, 0, TO_CHAR );
	return;
    }
    else
    {
	xp = 5 + GET_LEVEL(ch)*2;
	sprintf( buf,
	    "You get %d experience points for your sacrifice.", xp );
	act( "$n sacrifices $p to $s god.", FALSE, ch, obj, 0, TO_ROOM);
	act( buf, FALSE, ch, obj, 0, TO_CHAR );
	gain_exp(ch, xp, NULL);
	extract_obj(obj);
    }
}

void do_trip(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim=NULL;
    char name[256];
    byte percent;

    /* Mobs auto-trip in fight.c */
    if ( IS_NPC(ch) )
        return;
	if(ch->specials.stpMount){
		global_color=33;
		send_to_char("You think about dis-mounting and tripping..\n\r",ch);
		global_color=0;
		return;
	}
    if(in_a_shop(ch)){
        send_to_char("You realize it is not polite to fight in a public shop.\n\r",ch);
        return;
    }

    if ((GET_CLASS(ch) != CLASS_THIEF) && GET_LEVEL(ch)<33
	&&!IS_SET(ch->player.multi_class,MULTI_CLASS_THIEF)){
        send_to_char("You better leave all the agile fighting to thieves.\n\r",
		     ch);
        return;
    }

    one_argument(argument, name);

    victim = get_char_room_vis( ch, name );
    if ( victim == NULL )
        victim = ch->specials.fighting;

    if ( victim == NULL )
    {
        send_to_char( "Trip whom?\n\r", ch );
        return;
    }

    if (victim == ch) {
        send_to_char("Aren't we funny today...\n\r", ch);
        return;
    }
	if(IS_SET(victim->denyclass,DENYTRIP)){
		act("It's impossible to trip $N.",TRUE,ch,0,victim,TO_CHAR);
		return;
	}
    if(!if_allowed_to_attack(ch,victim))return;
    if (is_in_safe(ch,victim)==TRUE){
        act("$n tries to MURDER $N in a safe room!",
        FALSE, ch, 0, victim, TO_ROOM); 
        return; 
    }
   
    if((GET_MOB_POS(victim)==POSITION_BELLY)
		||(GET_POS(victim)==POSITION_SITTING)
		||(GET_MOB_POS(victim)==POSITION_SITTING)
		||(GET_MOB_POS(victim)==POSITION_BACK))
		{
		send_to_char("Your victim is already down.\n\r",ch);
        WAIT_STATE(ch, PULSE_VIOLENCE);
		return;	
		}

     /* 101% is a complete failure */
    percent=number(1,101) + (GET_LEVEL(victim) - GET_LEVEL(ch));
    if(!IS_IN_FRONT(victim)){
	act("You try and trip $M but $E is not close enough in formation.",TRUE,ch,0,victim,TO_CHAR);
	return;
    }
    if(!IS_IN_FRONT(ch))
	{
	send_to_char("You must move up in formation to trip...\n\r",ch);
	return;
    	}
    if (percent > ch->skills[SKILL_TRIP].learned * 2 / 3 ) 
	{
	act("$n tries to trip you, but fails.",
		FALSE, ch, NULL, victim, TO_VICT);
	act("You try to trip $N, but $E leaps out of the way.",
		FALSE, ch, NULL, victim, TO_CHAR);

        if(damage(ch, victim, 0, SKILL_TRIP))
			return;
        if(!IS_NPC(ch))
                WAIT_STATE(ch, PULSE_VIOLENCE);
        else
                GET_MOB_WAIT(ch)+=1;
	return;
	}
    else {
      act( "$n trips you and you go down!",
	  FALSE, ch, NULL, victim, TO_VICT );
      act( "You trip $N and $N goes down!",
	  FALSE, ch, NULL, victim, TO_CHAR );
      act( "$n trips $N and $E goes down!",
	  FALSE, ch, NULL, victim, TO_NOTVICT );
      if(damage(ch, victim, 1, SKILL_TRIP))
		return;
      WAIT_STATE(ch, PULSE_VIOLENCE*2);
      WAIT_STATE(victim, PULSE_VIOLENCE*2);
	  if(IS_NPC(victim))
		{
      	GET_MOB_POS(victim) = POSITION_SITTING;
      	if(number(1,100)<50)GET_MOB_POS(victim)=POSITION_BELLY;
        else GET_MOB_POS(victim)=POSITION_BACK;
		}
	  else
      	GET_POS(victim) = POSITION_SITTING;

	  fall(victim);

    }

}

void do_disarm( struct char_data *ch, char *argument, int cmd )
{
    struct char_data *victim=NULL;
    char name[256];
    int percent;
    struct obj_data *obj=NULL;

    /* Mobs auto-disarm in fight.c */
    if ( IS_NPC(ch) )
	return;

    if ( GET_CLASS(ch) != CLASS_WARRIOR && GET_CLASS(ch) != CLASS_THIEF
	    && GET_LEVEL(ch) < 33 
 		&&!IS_SET(ch->player.multi_class,MULTI_CLASS_WARRIOR)
		&&!IS_SET(ch->player.multi_class,MULTI_CLASS_THIEF))
    {
		send_to_char( "You don't know how to disarm opponents.\n\r", ch );
		return;
    }

    if ( ch->equipment[WIELD] == NULL ){
		send_to_char( "You must wield a weapon to disarm.\n\r", ch );
		return;
    }
    one_argument( argument, name );
    victim = get_char_room_vis( ch, name );
    if ( victim == NULL )
		victim = ch->specials.fighting;
    if ( victim == NULL ){
		send_to_char( "Disarm whom?\n\r", ch );
		return;
    }
    if ( victim == ch ){
		act( "$n disarms $mself!", FALSE, ch, NULL, victim, TO_NOTVICT );
		send_to_char( "You disarm yourself!\n\r", ch );
		obj = unequip_char( ch, WIELD );
		obj_to_room( obj, ch->in_room );
		return;
    }
    if(GET_MOB_POS(victim)==POSITION_BELLY)return;
    if ( victim->equipment[WIELD] == NULL ){
		send_to_char( "Your opponent is not wielding a weapon!\n\r", ch );
		return;
    }
    if(!if_allowed_to_attack(ch,victim))return;
    if (is_in_safe(ch,victim)==TRUE){
       act("$n tries to MURDER $N in a safe room!",
           FALSE, ch, 0, victim, TO_ROOM); 
       return; 
    }
    if(!IS_IN_FRONT(ch)){
		send_to_char("You cannot get around the person in front of you.\n\r",ch);
		return;
    }
    if(!IS_IN_FRONT(victim)){
		act("Your victim has someone in front of $M.",TRUE,ch,0,victim,TO_CHAR);
		return;
    }
	if(IS_SET(victim->denyclass,DENYDISARM)){
		act("$N easily deflects your weapon.",TRUE,ch,0,victim,TO_CHAR);
		return;
	}
    percent = number( 1, 100 ) + GET_LEVEL(victim) - GET_LEVEL(ch);
    if ( percent < ch->skills[SKILL_DISARM].learned * 2 / 3 )
    {
		disarm( ch, victim );
		one_hit( victim, ch, TYPE_UNDEFINED );
		WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    }else{
		one_hit( victim, ch, TYPE_UNDEFINED );
		WAIT_STATE( ch, PULSE_VIOLENCE*1 );
    }
}

void do_title(struct char_data *ch, char *argument, int cmd){
    char buf[100];
    char my_buf[100];
    int x=0, y=0;

    if (!*argument){
    	send_to_char("Change your title to what?\n\r", ch);
    	return;
    }
    
    for (; isspace(*argument); argument++);

    if (strlen(argument)>40){
    	send_to_char("Title field too big.  40 characters max.\n\r", ch);
    	return;
    }

    memset(my_buf,0,100);

    while(argument[x]!='\0')
	if(argument[x]=='~')
	    x++;
	else
	    my_buf[y++]=argument[x++];

    my_buf[y]='\0';
    ch->player.title = my_free(ch->player.title);

    if(strlen(argument)>2){
	ch->player.title = str_dup(my_buf);
	sprintf(buf,"Your title is now: %s\n\r", my_buf);
	send_to_char(buf,ch);
    }else 
	ch->player.title=str_dup("NOTITLE");
}

void do_split(struct char_data *ch, char *argument, int cmd)
{
  int amount,canhold=0,x,y;
  char buf[256], number[10];
  int no_members, share, extra;
  struct char_data *k=NULL;


  if (!*argument){
    send_to_char("Split what?\n\r", ch);
    return;
  }

  one_argument (argument, number);
  
  if (strlen(number)>7){
    send_to_char("Number field too big.\n\r", ch);
    return;
  }

  amount = atoi(number);

  if ( amount < 0 )
  {
    send_to_char( "Your formation wouldn't like that!\n\r", ch );
    return;
  }

  if ( amount == 0 )
  {
    send_to_char(
    "You hand out zero coins to everyone, but no one notices.\n\r", ch );
    return;
  }

  if (GET_GOLD(ch)<amount)
  {
    send_to_char( "You don't have that much gold!\n\r", ch );
    return;
  }
  
  if (ch->master!=ch)
    k = ch->master;
  else
    k = ch;

  if ( !is_formed(k) || k->in_room != ch->in_room )
  {
    send_to_char("You must be in a formation to split your money!\n\r", ch);
    return;
  }

  no_members = 0;
  for (x=0;x<3;x++)
  for (y=0;y<3;y++)
  {
    if(ch->master->formation[x][y])
    if(ch->master->formation[x][y]->in_room == ch->in_room)
      no_members++;
  }

  share = amount / no_members;
  extra = amount-(share*no_members);
  if(extra<1)extra=0;
  GET_GOLD(ch) -= amount;
  GET_GOLD(ch)+=extra;
  for (x=0;x<3;x++)
  for (y=0;y<3;y++)
  {
    if(ch->master->formation[x][y])
    if (ch->master->formation[x][y]->in_room == ch->in_room) {
      if (ch->master->formation[x][y]==ch)
	sprintf( buf, "You split %d gold coins.", amount );
      else 
	sprintf( buf, "%s splits %d gold coins.", GET_NAME(ch), amount );
      send_to_char( buf, ch->master->formation[x][y] );

     
    
if((GET_GOLD(ch->master->formation[x][y])+share)<=((GET_LEVEL(ch->master->formation[x][y])*GET_LEVEL(ch->master->formation[x][y]))*5555)){
         sprintf( buf, "Your share is %d gold coins.\n\r", share );
         send_to_char( buf, ch->master->formation[x][y] );
         GET_GOLD(ch->master->formation[x][y]) += share;
      }else{
        
	
if(GET_GOLD(ch->master->formation[x][y])>=((GET_LEVEL(ch->master->formation[x][y])*GET_LEVEL(ch->master->formation[x][y]))*5555)){
	    send_to_char("You simply can't hold any more gold.\n\r",ch->master->formation[x][y]);
	    act("$N can't hold any more gold.",FALSE,ch,0,ch->master->formation[x][y],TO_CHAR);
	    GET_GOLD(ch)+=share;
         }else{
	   
canhold=((GET_LEVEL(ch->master->formation[x][y])*GET_LEVEL(ch->master->formation[x][y]))*5555)-GET_GOLD(ch->master->formation[x][y]);
	    sprintf(buf,"%s takes %d coins, all $e can carry.\n\r",GET_NAME(ch->master->formation[x][y]),canhold);
 	    send_to_char(buf,ch);
  	    act(buf,FALSE,ch,0,ch,TO_CHAR);	
	    sprintf(buf,"You take %d coins, all you can carry.\n\r", canhold);
	    send_to_char(buf,ch->master->formation[x][y]);
	    GET_GOLD(ch->master->formation[x][y])+=canhold;
	    GET_GOLD(ch)+=(share-canhold);
         }     
      }
    } 
  }
}

void do_grouptell(struct char_data *ch, char *argument, int cmd)
{
  char buf[256];
  int x,y;

  if (!*argument){
    send_to_char("Tell your formation what?\n\r", ch);
    return;
  }

  for (; isspace(*argument); argument++);

  if (!is_formed(ch))
    {
      send_to_char("You don't have a formation to talk to!\n\r", ch);
      return;
    }

  global_color=36;
  sprintf(buf,"%s tells the formation, '%s'.\n\r",GET_NAME(ch),argument);
  for(x=0;x<3;x++)
  for(y=0;y<3;y++) 
  {
	if(ch->master->formation[x][y])
       		send_to_char(buf,ch->master->formation[x][y]);
  }
  global_color=0;
}

void do_report(struct char_data *ch, char *argument, int cmd)
{
  char buf[256];
  global_color=31;
  sprintf(buf, "$n says, 'My stats are:[%d/%dhp %d/%dm %d/%dmv %dac %dal]'",
      GET_HIT(ch),
      GET_MAX_HIT(ch),
      GET_MANA(ch),
      GET_MAX_MANA(ch),
      GET_MOVE(ch),
      GET_MAX_MOVE(ch),
      GET_AC(ch),
      ch->specials.alignment);
  act(buf, FALSE, ch, 0, 0, TO_CHAR);
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  global_color=0;
}

bool CAN_SEE( struct char_data *sub, struct char_data *obj )
{

	if (GET_LEVEL(sub) > 31)
		return TRUE;

int x,y;
    if (obj == sub)
      return TRUE;

/*  let players who are grouped with god see anyone in the group */

    if(is_formed(sub)){
	for(x=0;x<3;x++)
	    for(y=0;y<3;y++){
		if(sub->master)
			if(sub->master->formation[x][y]==obj)
		    	return TRUE;
	    }    
    }
    if ( obj->specials.wizInvis )
    {
	if ( IS_NPC(sub) || GET_LEVEL(sub) < GET_LEVEL(obj) )
	    return FALSE;
    }

    if ( sub->specials.holyLite )
	return TRUE;

    if ( IS_AFFECTED(sub, AFF_BLIND) )
	return FALSE;

    if ( !IS_LIGHT(sub->in_room) && !IS_AFFECTED(sub, AFF_INFRARED) )
	return FALSE;
/*
    if (IS_NPC(sub) && IS_AFFECTED(sub, AFF_SENSE_LIFE))
	return TRUE;
*/

/*
    if (  ( IS_AFFECTED(obj, AFF_SNEAK) || ( IS_AFFECTED(obj, AFF_HIDE) ) )
	&& IS_NPC(sub) && !IS_UNDEAD(sub) )
*/
    if( IS_AFFECTED(obj, AFF_HIDE) && IS_NPC(sub) && !IS_UNDEAD(sub) )
    {
	if (number(1,100)>=(1+(14*(MIN(35,GET_LEVEL(sub))-GET_LEVEL(obj)+30)/64)))
		return FALSE;
    }

    if ( !IS_AFFECTED(obj, AFF_INVISIBLE) )
	return TRUE;

    if ( IS_AFFECTED(sub, AFF_DETECT_INVISIBLE) )
	return TRUE;

    return FALSE;
}



bool CAN_SEE_OBJ( struct char_data *sub, struct obj_data *obj )
{
    if ( sub->specials.holyLite )
	return TRUE;

    if ( IS_AFFECTED( sub, AFF_BLIND ) )
	return FALSE;

    if ( !IS_LIGHT(sub->in_room) && !IS_AFFECTED(sub, AFF_INFRARED) )
	return FALSE;

    if ( !IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE) )
	return TRUE;

    if ( IS_AFFECTED(sub, AFF_DETECT_INVISIBLE) )
	return TRUE;

    return FALSE;
}



bool check_blind( struct char_data *ch )
{
    if ( IS_AFFECTED(ch, AFF_BLIND) )
    {
	send_to_char( "You can't see a damn thing!\n\r", ch );
	return TRUE;
    }

    return FALSE;
}



/*
 * Given a mob, determine what level its eq is.
 * Wormhole to shopping_buy.
 */
int map_eq_level( struct char_data *mob )
{
    if ( mob_index[mob->nr].func == shop_keeper )
	return 6;
    if ( GET_LEVEL(mob) <= 10 )
	return 0;
    else 
	return GET_LEVEL(mob) - 6;
}



void do_tick( struct char_data *ch, char *argument, int cmd )
{
    int	ntick;
    char buf[256];

    if ( IS_NPC(ch) )
    {
	send_to_char( "Monsters don't wait for anything.\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;

    while ( *argument == ' ' )
	argument++;

    if ( *argument == '\0' )
	ntick = 1;
    else
	ntick = atoi( argument );

    if ( ntick == 1 )
	sprintf( buf, "$n is waiting for one tick." );
    else
	sprintf( buf, "$n is waiting for %d ticks.", ntick );

    act(buf, TRUE, ch, 0, 0, TO_CHAR);
    act(buf, TRUE, ch, 0, 0, TO_ROOM);

    ch->desc->tick_wait	= ntick;
}

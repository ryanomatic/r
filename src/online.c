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
#include <string.h> 
#include <memory.h> 
#include <ctype.h> 
#include <time.h> 
#include "structs.h"
#include "mob.h" 
#include "obj.h" 
#include "utils.h" 
#include "db.h"
#include "handler.h" 
#include "limits.h" 
#include "spells.h"
#include "interp.h"


/*                  /External variables and functions\                    */
/*------------------------------------------------------------------------*/
extern struct room_data *world[MAX_ROOM];  
extern struct char_data *mobs[MAX_MOB];  
extern struct obj_data *objs[MAX_OBJ];  
extern FILE *hold2;
extern int top_of_world;  
extern struct obj_data *object_list;  
extern struct char_data *character_list;  
extern char global_color;  
extern struct zone_data zone_table_array[MAX_ZONE];  
extern struct zone_data *zone_table;  
extern int top_of_zone_table;  
extern struct index_data mob_index_array[MAX_MOB]; 
extern struct index_data *mob_index;  
extern struct index_data obj_index_array[MAX_OBJ];  
extern struct index_data *obj_index;  
extern int top_of_mobt;  
extern int top_of_objt; 
extern struct descriptor_data *descriptor_list;  
extern int dice(int number, int size);  
extern int number(int from, int to); 
extern void space_to_underline(char *text);
extern void page_string(struct descriptor_data *d, char *str, int keep_internal); 
extern int number_of_rooms;
extern char wmfha[1300];
extern struct room_actions *ractions[MAX_RACTION];
			/* local function prototypes */
int create_room(struct char_data *ch, int dir, int from_room);
void write_filtered_text(FILE *fh, char *text);


void load_room_actions(void)
{
struct room_actions *ra=NULL;
FILE *fh;
int x;

    if (!(fh = fopen("../lib/medievia.act", "r")))
    	{
        perror("medievia.act");
        return;;
    	}
	open_files++;
    while(1){
	fscanf(fh," action_num-%d \n", &x);
	if(x<MAX_RACTION){
	    fprintf(stderr,".");
	    CREATE(ra, struct room_actions, 1);
	    ractions[x]=ra;
	    ractions[x]->action_num=x;
	    fscanf(fh," type-%d \n",&ractions[x]->type);
	    fscanf(fh," in_room-%d \n",&ractions[x]->in_room);
	    fscanf(fh," chance-%d \n",&ractions[x]->chance);
	    fscanf(fh," arg1-%ld \n",&ractions[x]->arg1);
	    fscanf(fh," arg2-%ld \n",&ractions[x]->arg2);
	    fscanf(fh," arg3-%ld \n",&ractions[x]->arg3);
	    fscanf(fh," method-%d \n",&ractions[x]->method);
	    fscanf(fh," fired_next-%d \n",&ractions[x]->fired_next);
	    fscanf(fh," notfired_next-%d \n",&ractions[x]->notfired_next);
	    fscanf(fh," autostart-%d \n",&ractions[x]->autostart);
	    ractions[x]->text1=fread_string(fh);
	    ractions[x]->text2=fread_string(fh);
	    ractions[x]->text3=fread_string(fh);
sprintf(log_buf,"(num=%d)",x);
fprintf(stderr,log_buf);
	}else{
	    fclose(fh);
		open_files--;
    	fprintf(stderr,"Done\n");
	    return;
	}
    }
}

void save_room_actions(struct char_data *ch)
{
FILE *fh;
int x,y;

    if (!(fh = med_open("../lib/medievia.act", "w"))){
	send_to_char("Could not open file!\n\r",ch);
	return;
    }
    open_files++;
    y=0;
    for(x=0;x<MAX_RACTION;x++){
	if(ractions[x]){
	    y++;
	    fprintf(fh,"action_num-%d\n",ractions[x]->action_num);
	    fprintf(fh,"type-%d\n",ractions[x]->type);
	    fprintf(fh,"in_room-%d\n",ractions[x]->in_room);
	    fprintf(fh,"chance-%d\n",ractions[x]->chance);
	    fprintf(fh,"arg1-%ld\n",ractions[x]->arg1);
	    fprintf(fh,"arg2-%ld\n",ractions[x]->arg2);
	    fprintf(fh,"arg3-%ld\n",ractions[x]->arg3);
	    fprintf(fh,"method-%d\n",ractions[x]->method);
	    fprintf(fh,"fired_next-%d\n",ractions[x]->fired_next);
	    fprintf(fh,"notfired_next-%d\n",ractions[x]->notfired_next);
	    fprintf(fh,"autostart-%d\n",ractions[x]->autostart);
	    if(ractions[x]->text1)
	    	write_filtered_text(fh,ractions[x]->text1);
	    else
		fprintf(fh,"~\n");
	    if(ractions[x]->text2)
	    	write_filtered_text(fh,ractions[x]->text2);
	    else
		fprintf(fh,"~\n");
	    if(ractions[x]->text3)
	    	write_filtered_text(fh,ractions[x]->text3);
	    else
		fprintf(fh,"~\n");
	}
    }
    fprintf(fh,"action_num-%d\n",MAX_RACTION);
    med_close(fh);
    open_files--;
    sprintf(log_buf,"Done, %d room actions saved to ../lib/medievia.act\n\r",y);
    send_to_char(log_buf,ch);
}


	/*	Open and write zonename.wld into the wld directory    */
void save_rooms(struct char_data *ch, int zone)
{
FILE *wld_fh;
char full_filename[255];
char filename[255];
struct extra_descr_data *extra_descr=NULL;
int room, dir, door;

    strcpy(filename,zone_table[zone].name);
    space_to_underline(filename);
    strcpy(full_filename,"../lib/wld/");
    strcat(full_filename,filename);
    if (!(wld_fh = med_open(full_filename, "w")))
		{
		if(ch==NULL)
			{
			sprintf(log_buf,"error saving zone %d.\n\r",zone);
			log_hd(log_buf);
			}
		else
			send_to_char("Error opening file",ch); 
		return;
    	}
	open_files++;
    for(room=0;room<MAX_ROOM;room++)
	if(world[room]&&world[room]->zone==zone){
	   fprintf(wld_fh,"#%d\n",room);
	   write_filtered_text(wld_fh,world[room]->name);
	   write_filtered_text(wld_fh,world[room]->description);
	   fprintf(wld_fh,"%d %d %d %d\n",world[room]->zone,world[room]->room_flags,world[room]->sector_type,world[room]->extra_flags);
	   fprintf(wld_fh,"%d %d %d %d\n",
		world[room]->class_restriction,
		world[room]->level_restriction,
		world[room]->align_restriction,
		world[room]->mount_restriction);
	   fprintf(wld_fh,"%d %d %d\n",
		world[room]->move_mod,
		world[room]->pressure_mod,
		world[room]->temperature_mod);
	   for(dir=0;dir<=5;dir++) /* write out doors */
		if(world[room]->dir_option[dir]){
		if(world[room]->dir_option[dir]->key!=1
			&&world[world[room]->dir_option[dir]->to_room]->zone!=197){/*means like DON'T SAVE THE DAMN DOOR! like code made ones..*/
		    fprintf(wld_fh,"D%d\n",dir);
		    write_filtered_text(wld_fh,world[room]->dir_option[dir]->general_description);
		    write_filtered_text(wld_fh,world[room]->dir_option[dir]->keyword);
		    write_filtered_text(wld_fh,world[room]->dir_option[dir]->exit);
		    write_filtered_text(wld_fh,world[room]->dir_option[dir]->entrance);
		    door=0;
		    if(IS_SET(world[room]->dir_option[dir]->exit_info,EX_ISDOOR)) 
		 	door=1;
		    if(IS_SET(world[room]->dir_option[dir]->exit_info,EX_PICKPROOF)) 
		 	door=2;
		    if(IS_SET(world[room]->dir_option[dir]->exit_info,EX_SECRET))
		 	door=3;
		    if(IS_SET(world[room]->dir_option[dir]->exit_info,EX_HIDDEN))
		 	door=4;
		    if(IS_SET(world[room]->dir_option[dir]->exit_info,EX_ILLUSION))
		 	door=5;
		    if(!door&&world[room]->dir_option[dir]->keyword)
			door=1;
		    fprintf(wld_fh,"%d %d %d\n",
			door,
			world[room]->dir_option[dir]->key,
			world[room]->dir_option[dir]->to_room);
		}
		}
	   extra_descr=world[room]->ex_description;
	   while(extra_descr){
		fprintf(wld_fh,"E\n");
		write_filtered_text(wld_fh,extra_descr->keyword);
	        write_filtered_text(wld_fh,extra_descr->description);
		extra_descr=extra_descr->next;
	   }
	   /*change me later  add writing of chatters and modifiers */
	   fprintf(wld_fh,"S\n");
	}
        fprintf(wld_fh,"#19999\n$~\n"); /* ble ble ble  Thats all folks!*/
	med_close(wld_fh);
	open_files--;
}

void write_filtered_text(FILE *fh, char *text)
{
char *filtered;
int x=0,f=0;

    if(text){
      CREATE(filtered,char,strlen(text)+3);
      while(text[x]){
  	  if(text[x]=='\r')x++;
	  else
	     filtered[f++]=text[x++];
      }
      filtered[f++]='~';filtered[f++]='\n';filtered[f]=MED_NULL;
      fprintf(fh,filtered);
      filtered = my_free(filtered);
    }else{
       fprintf(fh,"~\n");
    }
}

void do_makedoor(struct char_data *ch, char *argument, int cmd)
{
int dir=0,opposite_dir=0,room,to_room;
char direction[MAX_INPUT_LENGTH],number[MAX_INPUT_LENGTH];
log_hd("in do_makedoor");
   if(ch)   
   if(ch->specials.editzone>=0)
   if(ch->specials.editzone!=world[ch->in_room]->zone&&GET_LEVEL(ch)<35){
       send_to_char("You are not authorized to do that for this zone.\n\r",ch); 
       return; 
   }
    

   half_chop(argument, direction, number);

   if(!direction[0]||!number[0]){
	if(ch)
	send_to_char("Syntax> makeexit direction(n,s,e,w,u,d) room#\n\r",ch);
	return;
   }
   switch(direction[0]){
	case 'n':
	case 'N':
	    dir=0;
	    opposite_dir=2;
	    break;
	case 'e':
	case 'E':
	    dir=1;
	    opposite_dir=3;
	    break;
	case 's':
	case 'S':
	    dir=2;
	    opposite_dir=0;
	    break;
	case 'w':
	case 'W':
	    dir=3;
	    opposite_dir=1;
	    break;
	case 'u':
	case 'U':
	    dir=4;
	    opposite_dir=5;
	    break;
	case 'd':
	case 'D':
	    dir=5;
	    opposite_dir=4;
	    break;
	default:
	    send_to_char("Direction must be n,e,s,w,u or d\n\r",ch);
	    return;;
   }
   if(ch)
      room=ch->in_room;
   else
      room=cmd;
   to_room=atoi(number);
   if(to_room<1||to_room>MAX_ROOM){
	if(ch)
	send_to_char("Room# must be between 1 and MAX_ROOM.\n\r",ch);
	return;
   }
   if(!world[to_room]){
	if(ch)
	send_to_char("That room doesn't exist.\n\r",ch);
	return;
   }
   if(world[room]->dir_option[dir]){
	if(ch)
	send_to_char("A door already exists there, use editroom to change it.\n\r",ch);
	return;
   }
   if(world[room]->zone!=198&&world[to_room]->zone!=198)
   if(world[to_room]->dir_option[opposite_dir]){
	if(ch)
	send_to_char("A door already exists in opposite direction in that room, use deletedoor.\n\r",ch);
	return;
   }
/* juice 
 * added this so people can't make exits from their zone to another that 
 * they are unauthorized to edit
 */
   if(ch)
   if(ch->specials.editzone>=0)
   if(ch->specials.editzone!=world[to_room]->zone&&GET_LEVEL(ch)<35){
       send_to_char("You are not authorized to create a door leading to that zone.\n\r",ch); 
       return; 
   }
   CREATE(world[room]->dir_option[dir],struct room_direction_data, 1); 
   world[room]->dir_option[dir]->general_description = 0; 
   world[room]->dir_option[dir]->keyword = 0; 
   world[room]->dir_option[dir]->exit = 0; 
   world[room]->dir_option[dir]->entrance = 0;
   world[room]->dir_option[dir]->exit_info = 0;
   world[room]->dir_option[dir]->key=-1;
   if(!ch)/*called with no ch from room_actions(temp door)*/
   world[room]->dir_option[dir]->key=1;
   world[room]->dir_option[dir]->to_room=to_room;

   if(world[room]->zone!=198){
   CREATE(world[to_room]->dir_option[opposite_dir],struct room_direction_data,1); 
   world[to_room]->dir_option[opposite_dir]->general_description = 0; 
   world[to_room]->dir_option[opposite_dir]->keyword = 0; 
   world[to_room]->dir_option[opposite_dir]->exit = 0; 
   world[to_room]->dir_option[opposite_dir]->entrance = 0;
   world[to_room]->dir_option[opposite_dir]->exit_info = 0;
   world[to_room]->dir_option[opposite_dir]->key=-1;
   if(!ch)
   world[to_room]->dir_option[opposite_dir]->key=1;
   world[to_room]->dir_option[opposite_dir]->to_room=room;
   }
   global_color=31;
   if(ch)
   send_to_char("Door CREATED in BOTH directions, use editroom to edit them\n\r",ch); 
   global_color=0;
}

void do_deletedoor(struct char_data *ch, char *argument, int cmd)
{
}

void do_makeroom(struct char_data *ch, char *argument, int cmd)
{
int amount,dir=0,new_room=0,x,from_room,insert_room=0;
char direction[MAX_INPUT_LENGTH],number[MAX_INPUT_LENGTH];


   if(ch->specials.editzone>=0)
   if(ch->specials.editzone!=world[ch->in_room]->zone&&GET_LEVEL(ch)<35){
       send_to_char("You are not authorized to do that for this zone.\n\r",ch); 
       return; 
   }
   half_chop(argument, direction, number);
   if(!direction[0]){
	send_to_char("Syntax> makeroom direction(n,s,e,w,u,d) #tomake(optional)\n\r",ch);
	return;
   }
   switch(direction[0]){
	case 'n':
	case 'N':
	    dir=0;
	    break;
	case 'e':
	case 'E':
	    dir=1;
	    break;
	case 's':
	case 'S':
	    dir=2;
	    break;
	case 'w':
	case 'W':
	    dir=3;
	    break;
	case 'u':
	case 'U':
	    dir=4;
	    break;
	case 'd':
	case 'D':
	    dir=5;
	    break;
	default:
	    send_to_char("Direction must be n,e,s,w,u or d\n\r",ch);
	    return;
   }
   if(world[ch->in_room]->dir_option[dir]){
	insert_room=world[ch->in_room]->dir_option[dir]->to_room;
   }
   if(!number[0]){
	new_room=create_room(ch,dir,ch->in_room);
	if(new_room!=-1)
 	   sprintf(log_buf,"Room [%d] Created %s.\n\r",new_room,direction);
	else
	   sprintf(log_buf,"Error, room not created!\n\r");
	global_color=31;
	send_to_char(log_buf,ch);
	global_color=0;
   }else{
	amount=atoi(number);
	if(amount<=1||amount>25){
	    send_to_char("Amount of rooms must be between 2 and 25.\n\r",ch);
	    return;
	}
	from_room=ch->in_room;
	for(x=0;x<amount;x++){
	    new_room=create_room(ch,dir,from_room);
	    if(new_room==-1){
		send_to_char("Error creating the room.\n\r",ch);
		return;
	    }else{
		from_room=new_room;
 	        sprintf(log_buf,"Room [%d] Created %s.\n\r",new_room,direction);
		global_color=31;
		send_to_char(log_buf,ch);
		global_color=0;
	    }
	}
   }
   if(insert_room){ /* we are INSERTING a room, not adding a new nonlinked */
	    CREATE(world[new_room]->dir_option[dir],struct room_direction_data, 1); 
	    world[new_room]->dir_option[dir]->general_description = 0; 
	    world[new_room]->dir_option[dir]->keyword = 0; 
	    world[new_room]->dir_option[dir]->exit = 0; 
	    world[new_room]->dir_option[dir]->entrance = 0;
	    world[new_room]->dir_option[dir]->exit_info = 0;
	    world[new_room]->dir_option[dir]->key=-1;
	    world[new_room]->dir_option[dir]->to_room=insert_room;
	    if(dir==0)dir=2; /* switch directions; */
	    else if(dir==1)dir=3;
	    else if(dir==2)dir=0;
	    else if(dir==3)dir=1;
	    else if(dir==4)dir=5;
	    else if(dir==5)dir=4;
	    world[insert_room]->dir_option[dir]->to_room=new_room;


   }   
}

int create_room(struct char_data *ch, int dir, int from_room)
{
int new_room,tmp;

    for(new_room=0;new_room<MAX_ROOM;new_room++)
	if(!world[new_room])break;
    if(new_room==MAX_ROOM){
	send_to_char("No rooms left!\n\r",ch);
	return(-1);
    }
    CREATE(world[new_room], struct room_data, 1); 
    number_of_rooms++; 
    world[new_room]->number = new_room; 
    world[new_room]->name=str_dup(world[from_room]->name);
    world[new_room]->description=str_dup(world[from_room]->description);
	world[new_room]->stpFreight=NULL;
    world[new_room]->zone = world[from_room]->zone;
    world[new_room]->room_flags=world[from_room]->room_flags;
    world[new_room]->sector_type=world[from_room]->sector_type;
	world[new_room]->holox=0;
	world[new_room]->holoy=0;
    world[new_room]->funct = 0; 
    world[new_room]->contents = 0; 
    world[new_room]->people = 0; 
    world[new_room]->light = 0;
    world[new_room]->ex_description = 0; 
    world[new_room]->extra_flags=world[from_room]->extra_flags;
    world[new_room]->class_restriction=world[from_room]->class_restriction;
    world[new_room]->level_restriction=world[from_room]->level_restriction;
    world[new_room]->align_restriction=world[from_room]->align_restriction;
    world[new_room]->mount_restriction=world[from_room]->mount_restriction;
    world[new_room]->move_mod=world[from_room]->move_mod;
    world[new_room]->pressure_mod=world[from_room]->pressure_mod;
    world[new_room]->temperature_mod=world[from_room]->temperature_mod;
    for (tmp = 0; tmp <= 5; tmp++)
	world[new_room]->dir_option[tmp] = 0; 
    CREATE(world[from_room]->dir_option[dir],struct room_direction_data, 1); 
    world[from_room]->dir_option[dir]->general_description = 0; 
    world[from_room]->dir_option[dir]->keyword = 0; 
    world[from_room]->dir_option[dir]->exit = 0; 
    world[from_room]->dir_option[dir]->entrance = 0;
    world[from_room]->dir_option[dir]->exit_info = 0;
    world[from_room]->dir_option[dir]->key=-1;
    world[from_room]->dir_option[dir]->to_room=new_room;
    if(dir==0)dir=2; /* switch directions; */
    else if(dir==1)dir=3;
    else if(dir==2)dir=0;
    else if(dir==3)dir=1;
    else if(dir==4)dir=5;
    else if(dir==5)dir=4;
    CREATE(world[new_room]->dir_option[dir],struct room_direction_data, 1); 
    world[new_room]->dir_option[dir]->general_description = 0; 
    world[new_room]->dir_option[dir]->keyword = 0; 
    world[new_room]->dir_option[dir]->exit = 0; 
    world[new_room]->dir_option[dir]->entrance = 0;
    world[new_room]->dir_option[dir]->exit_info = 0;
    world[new_room]->dir_option[dir]->key=-1;
    world[new_room]->dir_option[dir]->to_room=from_room;
    return(new_room);
}


/* juice
 * do_roomcopy and do_undordesc assume that ch->player.short_descr is
 * unused for player characters and copies the old room desc there for
 * later retrieval if needed.
 */

void do_roomcopy(struct char_data *ch, char *argument, int cmd)
{
struct obj_data *paper=NULL,*obj,tmp;
char arg[MAX_STRING_LENGTH];
char flag[MAX_STRING_LENGTH];
int fromroom;

argument_interpreter(argument, arg, flag);
if (!*arg || (arg[0] != 'p' && !isdigit(*arg)) )
	{
	send_to_char("Syntax:\n\rroomcopy [paper|roomnumber <flags>]\n\r",ch);
	send_to_char("Examples:\n\r",ch);
	send_to_char("roomcopy paper -- Copies description from paper to current room.\n\r",ch);
	send_to_char("roomcopy 195 -- Copies description from room 195 to current room. \n\r",ch);
	send_to_char("roomcopy 195 flags -- Also copies name, flags, sector type, etc. \n\r",ch);
	return;
	}
if (arg[0] == 'p')
	{
	/* look to see if char is carrying a paper */
	paper = &tmp;
	paper->item_number = MED_NULL;
	for(obj=ch->carrying;obj;obj=obj->next_content)
		if( (obj->item_number==1205) || (obj->item_number==3034) )
			{
			paper = obj;
			break;
			}
	if (paper->item_number == MED_NULL)
		{
		send_to_char("You're not carrying any paper.\n\r",ch);
		return;
		}
	/* make sure paper has an action desc */
	if ( (!paper->action_description) 
		|| (strlen(paper->action_description)<5) )
		{
		send_to_char("The paper is blank. Description unchanged.\n\r", ch);
		return;
		}
	/* save the old room desc for later*/
	ch->player.short_descr = my_free(ch->player.short_descr);
	ch->player.short_descr = world[ch->in_room]->description;
	/* copy paper desc to room desc */
	world[ch->in_room]->description = str_dup(paper->action_description);
	send_to_char("Room description copied from paper.\n\r",ch);
	act("$n copies in a new description from the paper.",TRUE,ch,0,0,TO_ROOM);
	}
else /* copy from a room */
	{
	fromroom = atoi(arg);
	if(fromroom == ch->in_room) {
		send_to_char("Copying the same room over?\r\n",ch);
		return;
	}
	if ( (fromroom < 0) || (fromroom >=MAX_ROOM) || !world[fromroom] )
		{
		send_to_char("No such room!\n\r",ch);
		return;
		}
	ch->player.short_descr = my_free(ch->player.short_descr);
	ch->player.short_descr = world[ch->in_room]->description;
	world[ch->in_room]->description=str_dup(world[fromroom]->description);
	sprintf(log_buf,"Description from room %d copied to this room.\n\r", fromroom);
	send_to_char(log_buf,ch);
	if (flag[0] == 'f')
	   {
	   world[ch->in_room]->name = my_free(world[ch->in_room]->name);
	   world[ch->in_room]->name = str_dup(world[fromroom]->name);
	   world[ch->in_room]->room_flags = world[fromroom]->room_flags;
	   world[ch->in_room]->sector_type = world[fromroom]->sector_type;
	   world[ch->in_room]->extra_flags = world[fromroom]->extra_flags;
	   world[ch->in_room]->class_restriction = world[fromroom]->class_restriction;
	   world[ch->in_room]->level_restriction = world[fromroom]->level_restriction;
	   world[ch->in_room]->align_restriction = world[fromroom]->align_restriction;
	   world[ch->in_room]->mount_restriction = world[fromroom]->mount_restriction;
	   world[ch->in_room]->move_mod = world[fromroom]->move_mod;
	   world[ch->in_room]->pressure_mod = world[fromroom]->pressure_mod;
	   world[ch->in_room]->temperature_mod = world[fromroom]->temperature_mod;
	   sprintf(log_buf,"Name, sector type, flags and mods from room %d copied to this room.\n\r", fromroom);
	   send_to_char(log_buf,ch);
	   }
	sprintf(log_buf,"$n copies room attributes from %s.",world[fromroom]->name);
	act(log_buf,TRUE,ch,0,0,TO_ROOM);
	}
} /* end do_roomcopy */


void do_undordesc(struct char_data *ch, char *argument, int cmd)
{
char *tmp;

if ( !ch->player.short_descr 
	|| (ch->player.short_descr == NULL)
	|| (strlen(ch->player.short_descr)<5) 
	)
	{
	send_to_char("No room description to undo.\n\r",ch);
	return;
	}
else
	{
	tmp = ch->player.short_descr;
	ch->player.short_descr = world[ch->in_room]->description;
	world[ch->in_room]->description = tmp;
	send_to_char("Room description change undone.\n\r",ch);
	act("$n undoes the description.",TRUE,ch,0,0,TO_ROOM);
	}
}

################################
CC	= gcc
MAKE	= gmake
PROGNAME = medievia_online
################################
MYPROFILE = -DHASHTABLE -DMEDTHIEVIA 
#MYPROFILE = -DOLDLIB -DMEMDEBUG -DHASHTABLE
C1_FLAGS = $(CC) -g -c $(O_FLAGS)	# -Wall -pipe 
L1_FLAGS = $(O_FLAGS)
O_FLAGS	= # -march=pentiumpro
L2_FLAGS = -lm -lcrypt		# for FreeBSD/Linux
#L2_FLAGS = -lm -lcrypt -lsocket -lnsl /usr/ucblib/libucb.a	# for Solaris
################################

O_FILES = act_comm.o act_info.o act_move.o act_o1.o act_o2.o act_off.o \
          act_oth.o act_soc.o act_wiz.o act_wiz2.o\
	  board.o \
	  casino.o catacomb.o changes.o clan.o main.o const.o continent.o \
	  death.o det.o dragons.o \
      editor.o \
      fight.o files.o flying.o formatn.o fount.o \
      handler.o holocode.o holomob.o\
      interp.o \
      limits.o  locker.o\
      mob_diff.o mob_act.o mob_mage.o mob_cler.o mob_warr.o mob_thie.o \
      magic.o modify.o mail.o mobfile.o mount.o \
      nanny.o newspaper.o \
	  objects.o objfile.o online.o \
	  pigeon.o \
	  quake.o \
	  room_act.o room_afs.o \
	  save.o shop.o spec_ass.o spec_pro.o spells.o \
	  trading.o tweak.o\
	  utility.o \
	  weather.o

medievia: $(O_FILES)
	$(PURE) $(CC) $(L1_FLAGS) -o $(PROGNAME) $(O_FILES)  $(L2_FLAGS)
	chmod 770 $(PROGNAME) 

oldlib: $(O_FILES)
	MYPROFILE = -DOLDLIB
	gcc $(L1_FLAGS) -o $(PROGNAME) $(O_FILES) $(L2_FLAGS)
	chmod 770 $(PROGNAME)

debug : $(O_FILES)
	MYPROFILE = -DDEBUG
	gcc $(L1_FLAGS) -o $(PROGNAME) $(O_FILES) $(L2_FLAGS)
	chmod 770 medievia_online

pacifist: $(O_FILES)
	MYPROFILE = -DPACIFIST -DNEWLIB
	gcc $(L1_FLAGS) -o $(PROGNAME) $(O_FILES) $(L2_FLAGS)
	chmod 770 medievia_online

.PHONY: clean
clean : 
	-rm -rf medievia_online $(O_FILES)

act_comm.o : act_comm.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h 
	$(C1_FLAGS) $(MYPROFILE) act_comm.c

act_info.o : act_info.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h ./limits.h ./holocode.h ./trading.h
	$(C1_FLAGS) $(MYPROFILE) act_info.c 

act_move.o : act_move.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h 
	$(C1_FLAGS) $(MYPROFILE) act_move.c

act_o1.o : act_o1.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h 
	$(C1_FLAGS) $(MYPROFILE) act_o1.c

act_o2.o : act_o2.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h 
	$(C1_FLAGS) $(MYPROFILE) act_o2.c

act_off.o : act_off.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h ./limits.h 
	$(C1_FLAGS) $(MYPROFILE) act_off.c

act_oth.o : act_oth.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h
	$(C1_FLAGS) $(MYPROFILE) act_oth.c

act_soc.o : act_soc.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h 
	$(C1_FLAGS) $(MYPROFILE) act_soc.c

act_wiz.o : act_wiz.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h ./limits.h ./holocode.h
	$(C1_FLAGS) $(MYPROFILE) act_wiz.c

act_wiz2.o : act_wiz2.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h ./limits.h
	$(C1_FLAGS) $(MYPROFILE) act_wiz2.c

board.o : board.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./handler.h 
	$(C1_FLAGS) $(MYPROFILE) board.c

casino.o : casino.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h ./casino.h 
	$(C1_FLAGS) $(MYPROFILE) casino.c

catacomb.o : catacomb.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h 
	$(C1_FLAGS) $(MYPROFILE) catacomb.c

changes.o : changes.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h ./limits.h 
	$(C1_FLAGS) $(MYPROFILE) changes.c

clan.o : clan.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h ./limits.h 
	$(C1_FLAGS) $(MYPROFILE) clan.c

main.o : main.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h 
	$(C1_FLAGS) $(MYPROFILE) main.c

const.o : const.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./limits.h ./db.h 
	$(C1_FLAGS) $(MYPROFILE) const.c

continent.o : continent.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h \
  ./interp.h ./handler.h ./db.h ./spells.h ./limits.h 
	$(C1_FLAGS) $(MYPROFILE) continent.c

death.o : death.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h ./limits.h 
	$(C1_FLAGS) $(MYPROFILE) death.c

det.o : det.c ./structs.h ./mob.h ./obj.h ./utils.h ./handler.h ./db.h ./spells.h \
	./limits.h ./tweak.h
	$(C1_FLAGS) $(MYPROFILE) det.c
	
dragons.o : dragons.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h ./holocode.h ./dragon.h
	$(C1_FLAGS) $(MYPROFILE) dragons.c

editor.o : editor.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h 
	$(C1_FLAGS) $(MYPROFILE) editor.c

fight.o : fight.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./handler.h \
  ./interp.h ./db.h ./spells.h 
	$(C1_FLAGS) $(MYPROFILE) fight.c

files.o : files.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h ./handler.h \
  ./limits.h ./spells.h 
	$(C1_FLAGS) $(MYPROFILE) files.c

flying.o : flying.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h ./holocode.h
	$(C1_FLAGS) $(MYPROFILE) flying.c

formatn.o : formatn.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h 
	$(C1_FLAGS) $(MYPROFILE) formatn.c

fount.o : fount.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h 
	$(C1_FLAGS) $(MYPROFILE) fount.c

guild.o : guild.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h ./limits.h 
	$(C1_FLAGS) $(MYPROFILE) guild.c

handler.o : handler.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./interp.h ./holocode.h ./dragon.h
	$(C1_FLAGS) $(MYPROFILE) handler.c

holocode.o : holocode.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h ./holocode.h
	$(C1_FLAGS) $(MYPROFILE) holocode.c

holomob.o : holomob.c ./structs.h ./mob.h ./utils.h ./db.h ./interp.h ./handler.h
	$(C1_FLAGS) $(MYPROFILE) holomob.c

interp.o : interp.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./interp.h ./db.h \
  ./utils.h ./limits.h 
	$(C1_FLAGS) $(MYPROFILE) interp.c

limits.o : limits.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./limits.h ./utils.h \
  ./spells.h ./db.h 
	$(C1_FLAGS) $(MYPROFILE) limits.c

locker.o : locker.c ./locker.h
	$(C1_FLAGS) $(MYPROFILE) locker.c
magic.o : magic.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./spells.h \
  ./handler.h ./limits.h ./interp.h ./db.h ./holocode.h
	$(C1_FLAGS) $(MYPROFILE) magic.c

mail.o : mail.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./limits.h 
	$(C1_FLAGS) $(MYPROFILE) mail.c

mob_act.o : mob_act.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h 
	$(C1_FLAGS) $(MYPROFILE) mob_act.c

mob_cler.o : mob_cler.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h 
	$(C1_FLAGS) $(MYPROFILE) mob_cler.c

mob_diff.o : mob_diff.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h 
	$(C1_FLAGS) $(MYPROFILE) mob_diff.c

mob_mage.o : mob_mage.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h 
	$(C1_FLAGS) $(MYPROFILE) mob_mage.c

mob_thie.o : mob_thie.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h 
	$(C1_FLAGS) $(MYPROFILE) mob_thie.c

mob_warr.o : mob_warr.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h 
	$(C1_FLAGS) $(MYPROFILE) mob_warr.c

mobfile.o : mobfile.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h 
	$(C1_FLAGS) $(MYPROFILE) mobfile.c

modify.o : modify.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h 
	$(C1_FLAGS) $(MYPROFILE) modify.c

mount.o : mount.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h ./holocode.h
	$(C1_FLAGS) $(MYPROFILE) mount.c

nanny.o : nanny.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./db.h ./limits.h 
	$(C1_FLAGS) $(MYPROFILE) nanny.c

newspaper.o : newspaper.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h \
  ./db.h ./handler.h ./limits.h ./spells.h ./interp.h 
	$(C1_FLAGS) $(MYPROFILE) newspaper.c

objects.o : objects.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h ./limits.h 
	$(C1_FLAGS) $(MYPROFILE) objects.c

objfile.o : objfile.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h 
	$(C1_FLAGS) $(MYPROFILE) objfile.c

online.o : online.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h 
	$(C1_FLAGS) $(MYPROFILE) online.c

pigeon.o : pigeon.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h 
	$(C1_FLAGS) $(MYPROFILE) pigeon.c

quake.o : quake.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h 
	$(C1_FLAGS) $(MYPROFILE) quake.c


room_act.o : room_act.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h 
	$(C1_FLAGS) $(MYPROFILE) room_act.c

room_afs.o : room_afs.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h ./holocode.h
	$(C1_FLAGS) $(MYPROFILE) room_afs.c

save.o : save.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h ./handler.h \
  ./spells.h ./interp.h ./holocode.h
	$(C1_FLAGS) $(MYPROFILE) save.c

shop.o : shop.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./handler.h ./db.h \
  ./interp.h ./utils.h 
	$(C1_FLAGS) $(MYPROFILE) shop.c

spec_ass.o : spec_ass.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h 
	$(C1_FLAGS) $(MYPROFILE) spec_ass.c

spec_pro.o : spec_pro.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./interp.h \
  ./handler.h ./db.h ./spells.h ./limits.h 
	$(C1_FLAGS) $(MYPROFILE) spec_pro.c

spells.o : spells.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./interp.h ./spells.h ./handler.h 
	$(C1_FLAGS) $(MYPROFILE) spells.c

trading.o : trading.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./db.h \
  ./handler.h ./limits.h ./spells.h ./interp.h ./trading.h
	$(C1_FLAGS) $(MYPROFILE) trading.c

tweak.o: tweak.c
	$(C1_FLAGS) $(MYPROFILE) tweak.c

utility.o : utility.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h 
	$(C1_FLAGS) $(MYPROFILE) utility.c

weather.o : weather.c ./structs.h ./room_afs.h ./mob.h ./obj.h ./utils.h ./handler.h \
  ./interp.h ./db.h 
	$(C1_FLAGS) $(MYPROFILE) weather.c

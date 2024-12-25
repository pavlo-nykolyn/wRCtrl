# Author: Pavlo Nykolyn
# compiler compilation and its flags
CC = gcc
override CFLAGS += -Wall
# object files
objects = wRCtrl.o ctrl.o\
          parser.o
# search paths
# internal paths
src-paths = src-controller $\
            src-parser
header-paths = headers-controller $\
               headers-parser $\
               headers-utilities $\
               headers-error-information
# the variable VPATH contains system paths
VPATH = /usr/include $\
        /usr/lib/gcc/x86_64-linux-gnu/12/include $\
        /usr/include/x86_64-linux-gnu/curl
vpath %.c $(src-paths)
vpath %.h $(header-paths)
# folder of the executable
bin-path = bin
# folder of the object files
obj-path = obj
# search paths of the header files used in the recipes
searchPaths-headers-recipes = $(foreach aPath, $(header-paths),-iquote $(aPath))
searchPaths-obj-recipes = $(addprefix $(obj-path)/, $(objects))
# library options
libs = -lcurl

.DELETE_ON_ERROR :
$(bin-path)/wRCtrl : $(objects)
	$(CC) $(CFLAGS) -o $@ $(searchPaths-obj-recipes) $(libs)
$(bin-path)/wRCtrl : | $(bin-path)
$(bin-path) :
	-mkdir -p $@

# generating the object files
wRCtrl.o : wRCtrl.c $\
           ctrl.h $\
           stdio.h stdlib.h stdbool.h string.h ctype.h $\
           curl.h $\
           err_wrapper.h
	$(CC) $(CFLAGS) $(searchPaths-headers-recipes) -o ./$(obj-path)/wRCtrl.o -c $<
ctrl.o : ctrl.c $\
         ctrl.h $\
         stdio.h stdlib.h string.h $\
         curl.h $\
         parser.h $\
         constants.h err_wrapper.h
	$(CC) $(CFLAGS) $(searchPaths-headers-recipes) -o ./$(obj-path)/ctrl.o -c $<
parser.o : parser.c $\
           stdio.h string.h ctype.h $\
           parser.h parser_constants.h $\
           err_wrapper.h
	$(CC) $(CFLAGS) $(searchPaths-headers-recipes) -o ./$(obj-path)/parser.o -c $<

$(objects) : | $(obj-path)
$(obj-path) :
	-mkdir -p $(obj-path)
.PHONY : clean
clean :
	@-rm -f -r $(bin-path)
	@-rm -f -r $(obj-path)

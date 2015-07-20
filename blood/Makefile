CC=gcc
CFLAGS=-std=c99 -ggdb -O0

SUFFIX=.exe
OUT=blood$(SUFFIX)

.PHONY: watch run debug clean

MD5=md5sum
RUN=blood-run$(SUFFIX)
PID=blood-run.pid
watch:
	make $(OUT); kill `cat $(PID)` 2>/dev/null; cp $(OUT) $(RUN) ; { ./$(RUN) & }; echo $$! > $(PID); while true ; do make $(OUT) | grep -v "is up to date\|make\[1\]: .* directory"; test "$${PIPESTATUS[*]}" = "0 0" && $(MD5) $(OUT) $(RUN) | cut -c 1-32 | sort -u | wc -l | grep -q "^2$$" && { kill `cat $(PID)` 2>/dev/null; cp $(OUT) $(RUN) ; { ./$(RUN) & }; echo $$! > $(PID); } ; sleep 1 ; done # try to build, if succeed launch

run: $(OUT)
	$(GDB) ./$<

debug:
	make GDB=gdb clean run

clean:
	rm -f $(OUT) $(RUN) $(PID)

main.c: noise.h
$(OUT): main.c noise.c
	$(CC) -o $@ $(CFLAGS) $^


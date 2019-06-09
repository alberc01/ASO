#ifndef _TIEMPO_H_
#define _TIEMPO_H_
/*
En el archivo conf.txt se debera indicar en formato string el clk_id->

       CLOCK_REALTIME
              System-wide clock that measures real (i.e., wall-clock) time.   Setting
              this  clock requires appropriate privileges.  This clock is affected by
              discontinuous jumps in the system time (e.g., if the system administra‐
              tor  manually  changes  the  clock), and by the incremental adjustments
              performed by adjtime(3) and NTP.

       CLOCK_REALTIME_COARSE (since Linux 2.6.32; Linux-specific)
              A faster but less precise version of CLOCK_REALTIME.  Use when you need
              very  fast, but not fine-grained timestamps.  Requires per-architecture
              support, and probably also architecture support for this  flag  in  the
              vdso(7).

       CLOCK_MONOTONIC
              Clock  that  cannot  be  set  and  represents monotonic time since some
              unspecified starting point.  This clock is not affected by  discontinu‐
              ous  jumps  in the system time (e.g., if the system administrator manu‐
              ally changes the clock), but is affected by the incremental adjustments
              performed by adjtime(3) and NTP.

       CLOCK_MONOTONIC_COARSE (since Linux 2.6.32; Linux-specific)
              A  faster  but  less  precise version of CLOCK_MONOTONIC.  Use when you
              need very fast, but not fine-grained timestamps.   Requires  per-archi‐
              tecture  support,  and probably also architecture support for this flag
              in the vdso(7).

       CLOCK_MONOTONIC_RAW (since Linux 2.6.28; Linux-specific)
              Similar to CLOCK_MONOTONIC, but provides access to a raw hardware-based
              time  that is not subject to NTP adjustments or the incremental adjust‐
              ments performed by adjtime(3).

       CLOCK_BOOTTIME (since Linux 2.6.39; Linux-specific)
              Identical to CLOCK_MONOTONIC, except it also includes any time that the
              system  is  suspended.  This allows applications to get a suspend-aware
              monotonic clock without  having  to  deal  with  the  complications  of
              CLOCK_REALTIME,  which  may have discontinuities if the time is changed
              using settimeofday(2) or similar.

       CLOCK_PROCESS_CPUTIME_ID (since Linux 2.6.12)
              Per-process CPU-time clock (measures CPU time consumed by  all  threads
              in the process).

       CLOCK_THREAD_CPUTIME_ID (since Linux 2.6.12)
              Thread-specific CPU-time clock.
*/


/*• void start() tomará el instante actual como referencia para el contador.*/
void start();

/*• int pause() parará temporalmente la cuenta asociada al contador.
Devuelve el tiempo transcurrido desde el último resume().
Si el contador ya estaba en pausa, esta llamada se ignora.*/
int pause();

/*• void resume() continúa la cuenta tras una pause().
Si el contador no estaba en pausa, esta llamada se ignora.*/
void resume();

/*• int stop() para definitivamente el contador Devuelve el tiempo transcurrido desde la última llamada a resume() ( o start() si nunca se llamó a pause() ).*/
int stop();/*para el timer definitivamente y devuelve el tiempo en microsegundos*/

#endif

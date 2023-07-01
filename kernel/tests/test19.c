/*******************************************************************************
 * Test 19
 *
 * Test du clavier.
 ******************************************************************************/

#include "test19.h"

int cons_rd0(void *arg)
{
        unsigned long i;
        char buf[101];

        (void)arg;

        i = cons_read(buf, 100);
        buf[i] = 0;
        printf(". : %s\n", buf);
        return 0;
}

int cons_rdN(void *arg)
{
        unsigned long i;
        char buf[101];

        (void)arg;

        i = cons_read(buf, 100);
        buf[i] = 0;
        printf("%d : %s\n", 133 - getprio(getpid()), buf);
        return 0;
}

static void mega_cycles(int n)
{
        unsigned long long t1, t2;
        int i = 0;

        __asm__ __volatile__("rdtsc":"=A"(t1));
        for (i=0; i<n; i++) {
                do {
                        test_it();
                        __asm__ __volatile__("rdtsc":"=A"(t2));
                } while ((t2 - t1) < 1000000);
                t1 += 1000000;
        }
}

int test19(void *arg)
{
        char ch[101];
        int i;
        unsigned long j;
        int pid1, pid2, pid3, pid4;

        (void)arg;

        printf("cons_read bloquant, entrez des caracteres : ");
        j = cons_read(ch, 100); ch[j] = 0;
        printf("%lu chars : %s\n", j, ch);
        printf("Frappez une ligne de 5 caracteres : ");
        j = cons_read(ch,5); ch[5] = 0;
        printf("%lu chars : %s\n", j, ch);
        j = cons_read(ch,5);
        if (j != 0) printf("!!! j aurait du etre nul\n");
        assert(cons_read(ch, 0) == 0);

        printf("Les tests suivants supposent un tampon clavier de l'ordre  de 20 caracteres.\n"
               "Entrez des caracteres tant qu'il y a echo, frappez quelques touches\n"
               "supplementaires puis fin de ligne : ");
        j = cons_read(ch, 100); ch[j] = 0;
        printf("%lu chars : %s\n", j, ch);

        printf("Entrees sorties par anticipation. Frappez 4 lignes en veillant a ne pas\ndepasser la taille du tampon clavier : ");
        for (i=0; i<80; i++) {
                mega_cycles(200);
                printf(".");
        }
        printf("Fini.\n");

        pid1 = start(cons_rd0, 4000, 129, "cons_rd0", 0);
        pid2 = start(cons_rd0, 4000, 129, "cons_rd0", 0);
        pid3 = start(cons_rd0, 4000, 129, "cons_rd0", 0);
        pid4 = start(cons_rd0, 4000, 129, "cons_rd0", 0);

        waitpid(pid2, 0);
        waitpid(pid3, 0);
        waitpid(pid1, 0);
        waitpid(pid4, 0);

        printf("Entrees sorties par anticipation. Depassez maintenant la taille du tampon\n"
               "clavier : ");
        for (i=0; i<80; i++) {
                mega_cycles(200);
                printf(".");
        }
        printf("Fini.\n");

        pid1 = start(cons_rd0, 4000, 129, "cons_rd0", 0);
        pid2 = start(cons_rd0, 4000, 129, "cons_rd0", 0);
        pid3 = start(cons_rd0, 4000, 129, "cons_rd0", 0);
        pid4 = start(cons_rd0, 4000, 129, "cons_rd0", 0);

        waitpid(pid2, 0);
        waitpid(pid3, 0);
        waitpid(pid1, 0);
        waitpid(pid4, 0);

        printf("Enfin on teste que dans le cas d'entrees bloquantes, les processus sont servis\n"
               "dans l'ordre de leurs priorites. Entrez quatre lignes : ");

        pid1 = start(cons_rdN, 4000, 130, "cons_rdN", 0);
        pid2 = start(cons_rdN, 4000, 132, "cons_rdN", 0);
        pid3 = start(cons_rdN, 4000, 131, "cons_rdN", 0);
        pid4 = start(cons_rdN, 4000, 129, "cons_rdN", 0);

        waitpid(pid2, 0);
        waitpid(pid3, 0);
        waitpid(pid1, 0);
        waitpid(pid4, 0);

        return 0;
}

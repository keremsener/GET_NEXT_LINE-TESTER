/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tester.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksener <ksener@student.42kocaeli.com.tr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 10:51:16 by ksener            #+#    #+#             */
/*   Updated: 2026/02/18 10:51:17 by ksener           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h> // for chmod

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 42
#endif

/* Prototype of the student's function */
char    *get_next_line(int fd);

/* COLORS & ICONS */
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

#define TICK    "✅"
#define CROSS   "❌"
#define WARN    "⚠️"

/* UTILS */
void    create_file(char *filename, char *content)
{
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) { perror("Error creating file"); exit(1); }
    if (content)
        write(fd, content, strlen(content));
    close(fd);
}

/* AUTOMATIC CHECKER FUNCTION */
void    check_result(char *line, char *expected, int line_num)
{
    if (!line && !expected)
    {
        printf("  Line %d: %s OK (NULL matches) %s\n", line_num, TICK, RESET);
    }
    else if (!line || !expected)
    {
        printf("  Line %d: %s FAIL (Null mismatch) %s\n", line_num, CROSS, RESET);
        if (line) printf("        Got: [%s]\n", line);
        else      printf("        Got: [NULL]\n");
    }
    else if (strcmp(line, expected) == 0)
    {
        printf("  Line %d: %s OK %s|%s|%s\n", line_num, TICK, GREEN, line, RESET);
    }
    else
    {
        printf("  Line %d: %s FAIL %s\n", line_num, CROSS, RESET);
        printf("        Expected: [%s]\n", expected);
        printf("        Got:      [%s]\n", line);
    }
}

/* ================= TESTS ================= */

void    test_basic(void)
{
    printf(CYAN "\n========================================\n");
    printf("       BASIC & EDGE CASE TESTS\n");
    printf("========================================\n" RESET);

    /* 1. Normal File */
    printf(BLUE "\n[TEST] Normal File (Kerem Sener Signature)\n" RESET);
    create_file("test_normal.txt", "Kerem\nSener\n42Global\n");
    int fd = open("test_normal.txt", O_RDONLY);
    char *line;
    
    line = get_next_line(fd); check_result(line, "Kerem\n", 1);    free(line);
    line = get_next_line(fd); check_result(line, "Sener\n", 2);    free(line);
    line = get_next_line(fd); check_result(line, "42Global\n", 3); free(line);
    line = get_next_line(fd); check_result(line, NULL, 4);         free(line);
    
    close(fd);
    unlink("test_normal.txt");

    /* 2. Newlines Only */
    printf(BLUE "\n[TEST] Newlines Only (\\n\\n\\n)\n" RESET);
    create_file("test_nl.txt", "\n\n\n");
    fd = open("test_nl.txt", O_RDONLY);
    
    line = get_next_line(fd); check_result(line, "\n", 1); free(line);
    line = get_next_line(fd); check_result(line, "\n", 2); free(line);
    line = get_next_line(fd); check_result(line, "\n", 3); free(line);
    line = get_next_line(fd); check_result(line, NULL, 4); free(line);
    
    close(fd);
    unlink("test_nl.txt");
}

void    test_hardcore_errors(void)
{
    printf(CYAN "\n========================================\n");
    printf("       HARDCORE ERROR HANDLING (UB ZONE)\n");
    printf("========================================\n" RESET);

    char *line;

    /* 1. Permission Denied File */
    /* What happens if the evaluator sets chmod 000? */
    printf(BLUE "\n[TEST] Permission Denied File (chmod 000)\n" RESET);
    create_file("test_locked.txt", "SecretKerem\n");
    chmod("test_locked.txt", 0000); // Remove read permissions
    
    int fd = open("test_locked.txt", O_RDONLY);
    
    /* Note: Depending on the OS/User, open() might fail (-1) OR 
       open() succeeds but read() fails. GNL must handle both.
    */
    if (fd == -1)
    {
         printf("  %s System blocked open() (Expected behavior). Testing invalid FD handling...\n", TICK);
         line = get_next_line(-1); // Test GNL with -1
    }
    else
    {
        // File opened but read might fail
        line = get_next_line(fd);
        close(fd);
    }

    if (line == NULL) printf("  %s OK: Returned NULL on protected file/error.\n", TICK);
    else { printf("  %s FAIL: Read from protected file? [%s]\n", CROSS, line); free(line); }
    
    unlink("test_locked.txt");


    /* 2. Read After Close (The Ghost File) */
    /* Trying to read from a file descriptor after closing it */
    printf(BLUE "\n[TEST] Read After Close (The Ghost File)\n" RESET);
    create_file("test_ghost.txt", "Boo\n");
    fd = open("test_ghost.txt", O_RDONLY);
    line = get_next_line(fd); // Reads "Boo\n"
    free(line);
    close(fd); // FILE CLOSED HERE!

    // Now try to read from the closed FD
    line = get_next_line(fd);
    if (line == NULL) printf("  %s OK: Returned NULL on closed FD.\n", TICK);
    else { printf("  %s FAIL: Read from closed FD! (Segfault risk) [%s]\n", CROSS, line); free(line); }
    unlink("test_ghost.txt");
}

void    test_stress_mix(void)
{
    printf(CYAN "\n========================================\n");
    printf("       STRESS MIX: LONG & SHORT LINES\n");
    printf("========================================\n" RESET);

    printf(BLUE "\n[TEST] Short -> HUGE -> Short (Memory Stress)\n" RESET);
    int fd = open("test_stress.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    
    // 1. Short line
    write(fd, "Start-Kerem\n", 12);
    
    // 2. Huge line (2000 chars of 'K')
    char *big_buffer = malloc(2001);
    memset(big_buffer, 'K', 2000);
    big_buffer[2000] = '\0';
    write(fd, big_buffer, 2000);
    write(fd, "\n", 1); // Newline
    free(big_buffer);

    // 3. Short line
    write(fd, "End-Sener\n", 10);
    close(fd);

    fd = open("test_stress.txt", O_RDONLY);
    
    // Check 1
    char *line = get_next_line(fd);
    check_result(line, "Start-Kerem\n", 1);
    if (line) free(line);

    // Check 2 (Huge)
    line = get_next_line(fd);
    if (line && strlen(line) == 2001) printf("  Line 2: %s OK (Length 2001 chars)\n", TICK);
    else printf("  Line 2: %s FAIL (Length mismatch or NULL)\n", CROSS);
    if (line) free(line);

    // Check 3
    line = get_next_line(fd);
    check_result(line, "End-Sener\n", 3);
    if (line) free(line);

    close(fd);
    unlink("test_stress.txt");
}

void    test_multi_fd(void)
{
    printf(CYAN "\n========================================\n");
    printf("       BONUS: MULTIPLE FD TEST\n");
    printf("========================================\n" RESET);

    create_file("test_m1.txt", "Kerem-1\nKerem-2\n");
    create_file("test_m2.txt", "Sener-1\nSener-2\n");
    create_file("test_m3.txt", "42-1\n42-2\n");

    int fd1 = open("test_m1.txt", O_RDONLY);
    int fd2 = open("test_m2.txt", O_RDONLY);
    int fd3 = open("test_m3.txt", O_RDONLY);

    char *l1, *l2;

    printf(MAGENTA "1. Reading FD1: " RESET);
    l1 = get_next_line(fd1);
    
    printf(MAGENTA "\n2. Reading FD2: " RESET);
    l2 = get_next_line(fd2);

    /* SMART CHECK: If bonus is not implemented, static variable will mix content */
    if (l2 && strstr(l2, "Kerem")) // If we got 'Kerem' instead of 'Sener'
    {
        printf(YELLOW "\n  %s WARNING: Bonus not active (Mixed content detected).\n", WARN);
        printf("  This is NORMAL if you only implemented the Mandatory part.\n" RESET);
    }
    else
    {
        check_result(l1, "Kerem-1\n", 1);
        check_result(l2, "Sener-1\n", 2);
    }

    if (l1) free(l1); if (l2) free(l2);
    close(fd1); close(fd2); close(fd3);
    unlink("test_m1.txt"); unlink("test_m2.txt"); unlink("test_m3.txt");
}

void    test_stdin(void)
{
    printf(CYAN "\n========================================\n");
    printf("       STDIN TEST\n");
    printf("========================================\n" RESET);
    printf(YELLOW "Run manual test with: echo \"KeremSener\" | ./gnl_tester manual\n" RESET);
}

void    test_stdin_manual(void)
{
    printf(CYAN "\n[TEST] Reading from STDIN\n" RESET);
    char *line;
    int i = 1;
    while ((line = get_next_line(0)) != NULL)
    {
        printf("  Line %d: %s%s%s", i++, GREEN, line, RESET);
        free(line);
    }
}

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "manual") == 0)
    {
        test_stdin_manual();
        return (0);
    }

    printf(GREEN "#####################################################\n");
    printf("   KEREM SENER - ULTIMATE HARDCORE GNL TESTER\n");
    printf("   BUFFER_SIZE: %d\n", BUFFER_SIZE);
    printf("#####################################################\n" RESET);

    test_basic();
    test_hardcore_errors();
    test_stress_mix();
    test_multi_fd();
    test_stdin();

    printf(GREEN "\n#####################################################\n");
    printf("   ALL TESTS COMPLETED\n");
    printf("#####################################################\n" RESET);
    
    return (0);
}

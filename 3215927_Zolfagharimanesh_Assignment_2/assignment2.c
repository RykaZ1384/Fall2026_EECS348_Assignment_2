/*
Program: EECS 348 Assignment 2

Description:
This program prioritizes emails for a CEO using a MaxHeap.
Emails are prioritized by sender category and then by date.

Input:
EMAIL, NEXT, READ, and COUNT commands from a test file.

Output:
Displays the next email and the number of unread emails.

Collaborators: None
Other Sources: Grok and Microsoft Copilot
Author: Raika Zolfagharimanesh
Student ID: 3215927
Creation Date: September 17, 2026
Revision Date: September 17, 2026

Revisions:
Used Grok code as the base and added comments and testing.
*/


#include <stdio.h> // access to input and output functions
#include <stdlib.h> // access to standard functions
#include <string.h> // access to string functions
#include <ctype.h> // access to character functions

#define MAX_EMAILS 10000 // maximum number of emails
#define MAX_SENDER 50 // maximum sender length
#define MAX_SUBJECT 256 // maximum subject length
#define MAX_DATE 20 // maximum date length
#define MAX_LINE 512 // maximum input line length

// store the information for each email
typedef struct {
    char sender[MAX_SENDER]; // store the sender
    char subject[MAX_SUBJECT]; // store the subject
    char date[MAX_DATE]; // store the date
    int priority; // store the sender priority
    int date_val; // store the date as a number
} Email;

// store the emails in a MaxHeap
typedef struct {
    Email data[MAX_EMAILS]; // store all the emails
    int size; // keep track of number of emails
} MaxHeap;

// give each sender a priority number
int get_priority(const char *sender) {
    if (strcmp(sender, "Boss") == 0) return 5; // Boss has highest priority
    if (strcmp(sender, "Subordinate") == 0) return 4; // Subordinate is second
    if (strcmp(sender, "Peer") == 0) return 3; // Peer is third
    if (strcmp(sender, "ImportantPerson") == 0) return 2; // ImportantPerson is fourth
    if (strcmp(sender, "OtherPerson") == 0) return 1; // OtherPerson has lowest priority
    return 0; // return 0 if sender is unknown
}

// change the date into a number so it is easier to compare
int parse_date(const char *date_str) {
    int month, day, year; // store month, day, and year

    if (sscanf(date_str, "%d-%d-%d", &month, &day, &year) != 3) { // check if the date was read correctly
        return 0; // return 0 if the date is invalid
    }

    return year * 10000 + month * 100 + day; // change the date into YYYYMMDD
}

// compare two emails to see which one has higher priority
int email_greater(const Email *a, const Email *b) {
    if (a->priority != b->priority) { // check if they have different sender priorities
        return a->priority > b->priority; // return the email with higher sender priority
    }

    return a->date_val > b->date_val; // if same priority then newer email comes first
}

// swap two emails
void swap_email(Email *a, Email *b) {
    Email tmp = *a; // temporarily store the first email
    *a = *b; // move the second email to the first spot
    *b = tmp; // move the first email to the second spot
}

// move an email up the heap
void heap_swim(MaxHeap *h, int idx) {
    while (idx > 0) { // keep going while the email is not at the top
        int parent = (idx - 1) / 2; // find the parent

        if (email_greater(&h->data[idx], &h->data[parent])) { // check if the email has higher priority than its parent
            swap_email(&h->data[idx], &h->data[parent]); // swap them
            idx = parent; // move to the parent position
        } else {
            break; // stop if the email is in the right position
        }
    }
}

// move an email down the heap
void heap_sink(MaxHeap *h, int idx) {
    int n = h->size; // store the heap size

    while (1) { // keep checking until the email is in the right position
        int left = 2 * idx + 1; // find the left child
        int right = 2 * idx + 2; // find the right child
        int largest = idx; // start with the current email

        if (left < n && email_greater(&h->data[left], &h->data[largest])) { // check the left child
            largest = left; // left child has higher priority
        }

        if (right < n && email_greater(&h->data[right], &h->data[largest])) { // check the right child
            largest = right; // right child has higher priority
        }

        if (largest == idx) break; // stop if the email is already in the right position

        swap_email(&h->data[idx], &h->data[largest]); // swap the emails
        idx = largest; // move down to the new position
    }
}

// add a new email to the heap
void heap_insert(MaxHeap *h, const Email *e) {
    if (h->size >= MAX_EMAILS) { // check if the heap is full
        return; // do not add another email if it is full
    }

    h->data[h->size] = *e; // add the email to the end
    heap_swim(h, h->size); // move the email to the right position
    h->size++; // increase the number of emails
}

// get the highest priority email without removing it
int heap_peek(const MaxHeap *h, Email *out) {
    if (h->size == 0) return 0; // return 0 if there are no emails

    *out = h->data[0]; // get the email at the top
    return 1; // return 1 if it worked
}

// remove the highest priority email
int heap_extract(MaxHeap *h, Email *out) {
    if (h->size == 0) return 0; // return 0 if there are no emails

    if (out) *out = h->data[0]; // save the top email if needed

    h->size--; // decrease the number of emails

    if (h->size > 0) { // check if there are still emails left
        h->data[0] = h->data[h->size]; // move the last email to the top
        heap_sink(h, 0); // move it to the right position
    }

    return 1; // return 1 if it worked
}

// remove extra spaces from the beginning and end
void trim(char *s) {
    char *start = s; // start at the beginning of the string

    while (*start && isspace((unsigned char)*start)) start++; // skip spaces at the beginning

    if (start != s) memmove(s, start, strlen(start) + 1); // move the string after removing beginning spaces

    size_t len = strlen(s); // get the length of the string

    while (len > 0 && isspace((unsigned char)s[len - 1])) { // check for spaces at the end
        s[--len] = '\0'; // remove spaces from the end
    }
}

// read the EMAIL command and separate its information
int parse_email_line(const char *line, Email *e) {
    const char *p = line; // start at the beginning of the line

    if (strncmp(p, "EMAIL", 5) != 0) return 0; // make sure the command is EMAIL

    p += 5; // move past the word EMAIL

    while (*p && isspace((unsigned char)*p)) p++; // skip spaces after EMAIL

    char buf[MAX_LINE]; // store a copy of the input
    strncpy(buf, p, sizeof(buf) - 1); // copy the input
    buf[sizeof(buf) - 1] = '\0'; // make sure the string ends correctly

    char *c1 = strchr(buf, ','); // find the first comma

    if (!c1) return 0; // stop if there is no first comma

    *c1 = '\0'; // separate the sender
    char *sender = buf; // store the sender
    trim(sender); // remove extra spaces from sender

    char *c2 = strchr(c1 + 1, ','); // find the second comma

    if (!c2) return 0; // stop if there is no second comma

    *c2 = '\0'; // separate the subject
    char *subject = c1 + 1; // store the subject
    trim(subject); // remove extra spaces from subject

    char *date = c2 + 1; // store the date
    trim(date); // remove extra spaces from date

    if (sender[0] == '\0' || date[0] == '\0') return 0; // make sure sender and date are not empty

    strncpy(e->sender, sender, MAX_SENDER - 1); // copy the sender into the email
    e->sender[MAX_SENDER - 1] = '\0'; // end the sender string

    strncpy(e->subject, subject, MAX_SUBJECT - 1); // copy the subject into the email
    e->subject[MAX_SUBJECT - 1] = '\0'; // end the subject string

    strncpy(e->date, date, MAX_DATE - 1); // copy the date into the email
    e->date[MAX_DATE - 1] = '\0'; // end the date string

    e->priority = get_priority(e->sender); // get the sender priority
    e->date_val = parse_date(e->date); // change the date into a number

    return 1; // return 1 if the email was read correctly
}

// main function where the program starts
int main(void) {
    MaxHeap heap = {0}; // create an empty heap
    char line[MAX_LINE]; // store each input line

    // read each command
    while (fgets(line, sizeof(line), stdin) != NULL) { // keep reading until there are no more commands

        size_t len = strlen(line); // get the length of the line

        if (len > 0 && line[len - 1] == '\n') { // check if there is a newline
            line[len - 1] = '\0'; // remove the newline
            len--; // decrease the length
        }

        if (len == 0) continue; // skip empty lines

        // check if the command is EMAIL
        if (strncmp(line, "EMAIL", 5) == 0) {
            Email e; // create an email

            if (parse_email_line(line, &e)) { // check if the email was read correctly
                heap_insert(&heap, &e); // add the email to the heap
            }
        }

        // check if the command is NEXT
        else if (strcmp(line, "NEXT") == 0) {
            Email top; // store the next email

            if (heap_peek(&heap, &top)) { // check if there is an email
                printf("Next email:\n"); // print the next email message
                printf("Sender: %s\n", top.sender); // print the sender
                printf("Subject: %s\n", top.subject); // print the subject
                printf("Date: %s\n", top.date); // print the date
            } else {
                printf("No emails to read.\n"); // tell the user there are no emails
            }
        }

        // check if the command is READ
        else if (strcmp(line, "READ") == 0) {
            heap_extract(&heap, NULL); // remove the highest priority email
        }

        // check if the command is COUNT
        else if (strcmp(line, "COUNT") == 0) {
            printf("There are %d emails to read.\n", heap.size); // print the number of emails left
        }
    }

    return 0; // end the program successfully
}


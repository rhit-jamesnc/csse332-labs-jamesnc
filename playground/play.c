#include <stdio.h>
#include <stdlib.h>

struct student {
    int age;
    int student_num;
};

int main(int argc, char* argv[]) {

    int* p = (int*)malloc(10 * sizeof(int));

    if (!p) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    p[0] = 1;
    p[1] = 2;

    // 10 ints is not enough! Lets grab 20 instead
    p = (int*)realloc(p, 20 * sizeof(int));
    
    struct student s1;

    s1.age = 18;
    s1.student_num = 1234;

    //create a student on the heap
    struct student* s2 =(struct student*)malloc(sizeof(struct student));

    if (!s2) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    s2->age = 19;
    s2->student_num = 1235;

    printf("First is %d and second is %d\n", p[0], p[1]);

    free(p);
    free(s2);

    return 0;
}
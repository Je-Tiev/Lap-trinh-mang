#include <stdio.h>

typedef struct student {
    char name[20];
    int eng;
    int math;
    int phys;
    double mean;
    char grade;
} STUDENT;

char determineGrade(double mean) {
    if (mean >= 90 && mean <= 100) return 'S';
    if (mean >= 80 && mean < 90) return 'A';
    if (mean >= 70 && mean < 80) return 'B';
    if (mean >= 60 && mean < 70) return 'C';
    return 'D';
}

int main() {
    STUDENT data[] = {
        {"Tuan", 82, 72, 58, 0.0, ' '},
        {"Nam", 77, 82, 79, 0.0, ' '},
        {"Khanh", 52, 62, 39, 0.0, ' '},
        {"Phuong", 61, 82, 88, 0.0, ' '}
    };
    
    int n = sizeof(data) / sizeof(data[0]);
    STUDENT *p;
    
    for(p = data; p < data + n; p++) {
        p->mean = (p->eng + p->math + p->phys) / 3.0;
        p->grade = determineGrade(p->mean);
    }
    
    for(p = data; p < data + n; p++) {
        printf("%-10s\t%d\t%d\t%d\t%.2f\t%c\n", 
            p->name, 
            p->eng, 
            p->math, 
            p->phys, 
            p->mean,
            p->grade);
    }
    
    return 0;
}

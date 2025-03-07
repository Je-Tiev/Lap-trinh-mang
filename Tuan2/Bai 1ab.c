#include <stdio.h>
struct student{
    char name[20];
    int eng;
    int math;
    int phys;
    double mean;
};

char determineGrade(double mean) {
    if (mean >= 90 && mean <= 100) return 'S';
    if (mean >= 80 && mean < 90) return 'A';
    if (mean >= 70 && mean < 80) return 'B';
    if (mean >= 60 && mean < 70) return 'C';
    return 'D';
}

int main()
{
    static struct student data[]={ 
        {"Tuan", 82, 72, 58, 0.0}, 
        {"Nam", 77, 82, 79, 0.0},  
        {"Khanh", 52, 62, 39, 0.0},  
        {"Phuong", 61, 82, 88, 0.0} 
    }; 
    
    int n = sizeof(data) / sizeof(data[0]);
    struct student *p;
    
    printf("\nKet qua diem va xep hang sinh vien:\n");
    printf("----------------------------------------\n");
    printf("Ten\t\tDiem TB\t\tXep hang\n");
    printf("----------------------------------------\n");
    
    for(p = data; p < data + n; p++) {
        p->mean = (p->eng + p->math + p->phys) / 3.0;
        printf("%-10s\t%.2f\t\t%c\n", 
            p->name, 
            p->mean,
            determineGrade(p->mean));
    }
    
    return 0;
}

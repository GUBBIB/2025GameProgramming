#include <stdio.h>

#define MAX 10

int stack[MAX];
int top = -1;

// push: 스택에 값 넣기
void push(int value) {
    if (top < MAX - 1) {
        stack[++top] = value;
    } else {
        printf("스택 오버플로우!\n");
    }
}

// pop: 스택에서 값 꺼내기
int pop(void) {
    if (top >= 0) {
        return stack[top--];
    } else {
        printf("스택 언더플로우!\n");
        return -1;
    }
}

int main(void) {
    push(10);
    push(20);
    push(30);

    printf("%d\n", pop()); // 30
    printf("%d\n", pop()); // 20
    printf("%d\n", pop()); // 10
    printf("%d\n", pop()); // 언더플로우 → -1

    return 0;
}

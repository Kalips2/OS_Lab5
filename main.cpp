#include <sys/msg.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>

struct message {
    long mtype;
    long mvalue;
};

void send_message(const int msgid,
                  const int result,
                  const long result_type) {
    message msg{};
    msg.mtype = result_type;
    msg.mvalue = result;
    printf("Sending message type %ld: %d\n", result_type, result);
    msgsnd(msgid, &msg, sizeof(msg.mvalue), 0);
}

int f(const int x) {
    sleep(1);
    return x * 2;
}

int g(const int x) {
    sleep(3);
    return x * 10;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <integer value>\n", argv[0]);
        return -1;
    }

    const int x = atoi(argv[1]);

    const int msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);

    if (fork() == 0) {
        // Start f(x) from main process
        const int result = f(x);
        send_message(msgid, result, 1);
        exit(0);
    }

    if (fork() == 0) {
        // Start g(x) from main process
        int result = g(x);
        send_message(msgid, result, 2);
        exit(0);
    }

    message msg{};
    int results[2] = {0}, count = 0;

    while (count < 2) {
        if (msgrcv(msgid, &msg, sizeof(msg.mvalue), 0, 0) > 0) {
            results[msg.mtype - 1] = msg.mvalue;
            count++;
        }
    }

    const int product = results[0] * results[1];

    printf("f(x) is %d\n", results[0]);
    printf("g(x) is %d\n", results[1]);
    printf("Result is %d\n", product);

    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}
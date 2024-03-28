#ifndef CANMSG_H
#define CANMSG_H

class CANMessage {

    public:
        CANMessage(uint32_t id, uint8_t *data, uint8_t len, bool extended, bool rtr, uint16_t timestamp) {
            this->id = id;
            this->len = len;
            this->extended = extended;
            this->rtr = rtr;
            this->timestamp = timestamp;
            for (int i = 0; i < len; i++) {
                this->data[i] = data[i];
            }
        }

        void print() {
            console.print("ID: ");
            console.println(id);
            console.print("Data: ");
            for (int i = 0; i < len; i++) {
                console.print(data[i]);
                console.print(" ");
            }
            console.println();
            console.print("Length: ");
            console.println(len);
            console.print("Extended: ");
            console.println(extended);
            console.print("RTR: ");
            console.println(rtr);
            console.print("Timestamp: ");
            console.println(timestamp);
        }
};

#endif
//acks are not numbered
#include <iostream>
#include <cstdlib>
#include <ctime>
#define SENDER 0
#define RECEIVER 1
#define TIME_OUT 5

int frame_to_send = 1;
int turn;
typedef enum { frame_arrival, error, time_out, send_request } event_type;
typedef struct { int seq_no, data, error; } frame;
bool finished = false;
frame buffer;

void wait_for_event_sender(event_type &);
void wait_for_event_receiver(event_type &);
int get_data_from_network_layer();
void send_data_to_network_layer(int);
frame get_frame_from_physical_layer();
void send_frame_to_physical_layer(frame);
void sender();
void receiver();
void increment(int &seq_no) { seq_no = (seq_no + 1) % 2; }

using namespace std;

int main()
{
	srand(time(NULL));
    while(!finished)
    {
    	sender();
    	receiver();
    }
    return 0;
}

void sender()
{
	static int next_seq_no = 0;
	static frame s;
	event_type event;
	static bool can_send = true; //for sending the first time
	if(can_send)
	{
		s.data = get_data_from_network_layer();
		s.seq_no = next_seq_no;
		cout << "SENDER: Sending frame " << s.data << ", sequence number: " << s.seq_no << "\t";
		turn = RECEIVER;
		send_frame_to_physical_layer(s);
		can_send = false; //from now on it is up to the user to send or not
	}
	wait_for_event_sender(event);
	if(turn == SENDER)
	{
		if(event == send_request)
		{
			increment(next_seq_no);
			s.data = get_data_from_network_layer();
			s.seq_no = next_seq_no;
			cout << "SENDER: Sending frame " << s.data << ", sequence number: " << s.seq_no << "\t";
			turn = RECEIVER;
			send_frame_to_physical_layer(s);
		}
		else if(event == time_out)
		{
			cout << "SENDER: Resending frame\t\t\t\t";
			turn = RECEIVER;
			send_frame_to_physical_layer(s);
		}
	}
}

void wait_for_event_sender(event_type &event)
{
	static int timer = 0;
	if(turn == SENDER)
	{
		timer++;
		if(timer == TIME_OUT)
		{
			event = time_out;
			cout << "SENDER: Ack not received -> Timeout\n";
			return;
		}
		if(buffer.error == 0) event = error;
		else
		{
			timer = 0;
			event = frame_arrival;
			cout << "SENDER: Ack received\n";
			//safely received the ack
			char request;
			cout << "Do you want to send another frame?: ";
			cin >> request;
			if(request == 'y' || request == 'Y') event = send_request;
			else finished = true;
		}
	}
}

void receiver()
{
	static int expected_frame_seq_no = 0;
	frame received;
	event_type event;
	wait_for_event_receiver(event);
	if(turn == RECEIVER)
  	{	
   		if(event == frame_arrival)
    	{
     		received = get_frame_from_physical_layer();
     		if(received.seq_no == expected_frame_seq_no)
      		{
       			send_data_to_network_layer(received.data);
       			increment(expected_frame_seq_no);
      		}
      		//else part: when the sender did not receive the ack, and resends the frame receiver has already 
      		//received, receiver will have to resend the acknowledgement 
     		else cout << "RECEIVER : Acknowledgement resent" << endl;
     		turn = SENDER;
     		frame s;
     		send_frame_to_physical_layer(s);
    	}
   		else if(event == error)
    	{
      		cout << "RECEIVER : Erroneous frame" << endl;
      		turn = SENDER;     
      	}
  	}
}

void wait_for_event_receiver(event_type &event)
{
	if(turn == RECEIVER)
  	{
   		if(buffer.error == 0) event = error;
   		else event = frame_arrival;
  	}
}

int get_data_from_network_layer() { return frame_to_send++; }
void send_data_to_network_layer(int data)
{
	cout << "RECIEVER: Frame " << data << " recieved, ack sent\n";
}
frame get_frame_from_physical_layer() { return buffer; }
void send_frame_to_physical_layer(frame s)
{
	s.error = rand() % 5;
	buffer = s;
}
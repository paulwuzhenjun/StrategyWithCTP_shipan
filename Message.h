#pragma once

class Message
{
public:
	//CString MessageType;
	char type;

private:
	unsigned char data[3000];

public:
	Message(void);
	~Message(void);

	void AddData(void* src, int rank, int length);
	void GetData(void* des, int rank, int length);

private:

	int GetLocation(int rank);
};

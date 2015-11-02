//This header file manages the objects used in the program

#ifndef _BARBER_SHOP_SIMULATION_H
#define _BARBER_SHOP_SIMULATION_H

using namespace std;

#define NUM_WAITING_CHAIRS 5

class Barber;
class Customer;
class BarberShop;

//Thread class, to make threading a little easier to understand in the program
class Thread
{
	HANDLE m_ThreadHandle;
	bool m_Terminate;
	DWORD m_ThreadID;

	static DWORD _ThreadProc(void *);
public:
	Thread();
	virtual ~Thread();
	void Start();
	void Stop() 
	{ 
		m_Terminate = true;
	}
	bool KeepRunning()
	{ 
		return !m_Terminate;
	}
	DWORD GetThreadID() const 
	{ 
		return m_ThreadID;
	}
	virtual DWORD run() = 0;
};

//Barber class
class Barber : public Thread
{
	BarberShop & m_Shop;

public:
	Barber(BarberShop & inShop);
	~Barber();
	DWORD run();
};

//Customer class
class Customer
{
	int m_ID;
public:
	Customer(int inID) : m_ID(inID){}
	int GetID() const  
	{ 
		return m_ID;
	}

	Customer(const Customer & inCust) : m_ID(inCust.m_ID){}
};

ostream & operator << (ostream & os, const Customer & inCust)
{
	os << inCust.GetID();
	return os;
}

//Barber shop class
class BarberShop
{

	auto_ptr<Barber> m_Barber;
	HANDLE m_BarberSleepEvent;
	HANDLE m_Mutex;
	int m_Head, m_Tail;
	auto_ptr<Customer>  m_WaitingCustomers[NUM_WAITING_CHAIRS];

	inline void incr(int& inValue)
	{
		inValue++;
		if (inValue >= NUM_WAITING_CHAIRS)
			inValue = 0;
	}
public:
	BarberShop();
	~BarberShop(){}

	void GetNextCusomer(std::auto_ptr<Customer> & outCust);
	void NeedsHaircut(std::auto_ptr<Customer> & inCust);
};
#endif
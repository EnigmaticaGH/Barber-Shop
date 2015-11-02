//Programmer: Derek Shunia/
//headers
#include <thread>
#include <future>
#include <iostream>
#include <string>
#include <vector>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "BarberClasses.h"

using namespace std;

//Thread creation
Thread::Thread() : m_ThreadHandle(NULL), m_Terminate(false){}

Thread::~Thread()
{
	if (m_ThreadHandle != NULL)
	{
		::TerminateThread(m_ThreadHandle, 0);
		m_ThreadHandle = NULL;
	}
}

DWORD Thread::_ThreadProc(void *ap)
{
	Thread *r = (Thread *)ap;
	return r->run();
}

void Thread::Start()
{
	m_ThreadHandle = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_ThreadProc, this, 0, &m_ThreadID);
	if (m_ThreadHandle == NULL)  cout << "Error creating a thread.";
}

//Barber creation
Barber::Barber(BarberShop & inShop) : m_Shop(inShop){}
Barber::~Barber(){}

DWORD Barber::run()
{
	cout << endl << "Barber begins cutting hair." << endl;
	while (true)
	{
		auto_ptr<Customer> customer;
		m_Shop.GetNextCusomer(customer);

		if (!customer.get())
		{
			cout << "Barber finds her customer ran off and decided to take the rest of the day off.";
			return 1;
		}
		cout << "    Barber is now cutting hair for customer " << *customer << endl;
		::Sleep(666);
		cout << "     Finished\n";
	}
}


//Barber shop creation
BarberShop::BarberShop() : m_BarberSleepEvent(NULL), m_Mutex(NULL), m_Head(0),  m_Tail(0)
{
	m_BarberSleepEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_Mutex = ::CreateMutex(NULL, FALSE, NULL);
	auto_ptr<Barber> barber(new Barber(*this));
	m_Barber = barber;
	m_Barber->Start();
}

void BarberShop::GetNextCusomer(auto_ptr<Customer>& outCust)
{
	DWORD result;

	do
	{
		result = ::WaitForSingleObject(m_Mutex, INFINITE);
		if (result != WAIT_OBJECT_0)
		{
			cout << "Error while waiting for Mutex!\n";
			return;
		}
		//If no customers are waiting, take a break. When a customer arrives, barbers will resume work
		if (m_Head ==  m_Tail)
		{
			::ReleaseMutex(m_Mutex);
			result = ::WaitForSingleObject(m_BarberSleepEvent, INFINITE);
			if (result != WAIT_OBJECT_0)
			{
				cout << "Error while waiting for event!\n";
				return;
			}
		}
		else
		{
			//There are some waiting customers. Cut his hair!
			outCust = m_WaitingCustomers[m_Head];
			incr(m_Head);
			::ReleaseMutex(m_Mutex);
			return;
		}
	} while (true);
}

void BarberShop::NeedsHaircut(auto_ptr<Customer> & inCust)
{
	DWORD result;
	int nextPos;
	bool emptyQueue;

	//Get the Mutex
	result = ::WaitForSingleObject(m_Mutex, INFINITE);
	if (result != WAIT_OBJECT_0)
	{
		cout << "Error while waiting for mutex!\n";
		return;
	}

	//Add customer to queue if the queue is not full
	emptyQueue =  m_Tail == m_Head;
	nextPos =  m_Tail;
	incr(nextPos);
	if (nextPos == m_Head)
	{
		cout << "Customer " << *inCust << " cannot enter, because the queue is full.\n";
		::ReleaseMutex(m_Mutex);
		return;
	}

	m_WaitingCustomers[m_Tail] = inCust;
	m_Tail = nextPos;
	::ReleaseMutex(m_Mutex);
	if (emptyQueue)
	{
		::SetEvent(m_BarberSleepEvent);
	}
	return;
}


//Main function
int main()
{
	cout << "Welcome to the barber shop." << endl << endl;
	int customers;
	BarberShop barberShop;

	for (int i = 1; i < 50;)
	{
		srand(time(NULL));
		customers = rand() % 4;
		for (int j = 0; j < customers; j++)
		{
			auto_ptr<Customer> customer(new Customer(i));
			cout << "  Adding customer #" << i << " to shop." << endl;
			barberShop.NeedsHaircut(customer);
			i++;
		}
		::srand(time(NULL));
		::Sleep(999);
	}

	::Sleep(99999);
	system("pause");
	return 0;
}
#include <iostream>
#include <cstdint>
#include <ctime>
#include <string>
using namespace std;

uint64_t canary_secret = 0; //int only holds 32b

// generate random canary value
void setup_canary()
{
	srand(time(nullptr)); //random number generator so the canary is different every time the program runs. 
	//If it were always the same, an attacker could just include the canary in their overflow string.

	//srand() generates a 32bit number only

	canary_secret = ((uint64_t)rand() << 32) | rand(); //shifts initial 32b value to the left so that the new value is added to the right

	if (canary_secret == 0) //safety check
	{
		canary_secret = 0xCAFEBABEDEADBEEFULL;
	}

	cout << "[SETUP] Canary secret created: 0x" << hex << canary_secret << dec << endl;
}

class CanaryGuard
{
	private:
		string function_name;
		uint64_t local_canary;
	public:
		CanaryGuard(const string& name)
		{
			function_name = name;
			local_canary = canary_secret;

			cout << "[ENTER] " << function_name << " | canary placed: 0x" << hex << local_canary << dec << endl;
		}

		~CanaryGuard()
		{
			if (local_canary != canary_secret)
			{
				cout << "\n[ALERT] Stack smashing detected in function: " << function_name << endl;
				cout << "[ALERT] Expected canary : 0x" << hex << canary_secret << endl;
				cout << "[ALERT] Found canary : 0x" << local_canary << dec << endl;
				cout << "[ALERT]  Program aborted. \b";
				exit(EXIT_FAILURE); //no function finishes normally text at the end
			}
			cout << "[EXIT] " << function_name << " | canary intact\n";
		}


		void corrupt_canary (uint64_t fake_value) //unsigned integer 64 type
		{
			cout << "[ATTACK] Corrupting canary \n";
			local_canary = fake_value;
		}
};

//Normal protected functions
void safe_function()
{
	CanaryGuard guard("safe_function");
	char buffer[32] = "Hello Secure OS!";
	cout << "[safe_function] Buffer contains: " << buffer << endl;
}


void normal_processing()
{
	CanaryGuard guard("normal_processing");
	string text = "Operating System Security";
	cout << "[normal_processing] Processing: " << text << endl;
}

// Attack Simulation
void attack_simulation()
{
	CanaryGuard guard("attack_simulation");
	cout << "[attack_simulation] simulation buffer overflow attack \n";

	//simulate overwriting the canary
	guard.corrupt_canary(0xBADBADBADBADBADULL);

	cout << "[attack_simulation] Returning from function \n";
}

int main()
{
	cout << "====Stack Canary Demo ====\n" << endl;
	setup_canary();

	cout << "--- Test Case 1: Normal ---" << endl;
	safe_function();

	cout << "--- Test Case 2: Normal ---" << endl;
	normal_processing();

	cout << "--- Test Case 3: Attack Simulation ---" << endl;
	attack_simulation();

	cout << "Program finished normally." << endl;

	return 0;
}
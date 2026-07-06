#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
using namespace std;

struct StackEntry
{
	void*  return_address;
	string function_name;
};

vector<StackEntry> shadow_stack;
int violation_count = 0;

void shadow_push(void* addr, const string& name)
{
	StackEntry entry;
	entry.return_address = addr;
	entry.function_name  = name;
	shadow_stack.push_back(entry);

	cout << "[PUSH]  " << name
	     << " | depth=" << shadow_stack.size()
	     << " | addr=" << addr << endl;
}

bool shadow_verify(void* actual_addr, const string& name)
{
	if (shadow_stack.empty())
	{
		cerr << "[ERROR] Shadow stack is empty!" << endl;
		exit(EXIT_FAILURE);
	}

	void* stored_addr = shadow_stack.back().return_address;
	shadow_stack.pop_back();

	cout << "[CHECK] " << name
	     << " | stored=" << stored_addr
	     << " | actual=" << actual_addr << endl;

	if (stored_addr != actual_addr)
	{
		violation_count++;
		cout << "\n[ALERT] Return address hijack detected in: " << name << endl;
		cout << "[ALERT] Stored address : " << stored_addr << endl;
		cout << "[ALERT] Actual address : " << actual_addr << endl;
		cout << "[ALERT] ROP attack blocked. Program aborted." << endl;
		return false;
	}

	cout << "[OK]    " << name << " | return address verified" << endl;
	return true;
}

class ShadowGuard
{
	private:
		string function_name;
		void*  saved_address;

	public:
		ShadowGuard(const string& name, void* caller_addr)
		{
			function_name = name;
			saved_address = caller_addr;
			shadow_push(saved_address, function_name); //built in command from the <vector> header
		}

		~ShadowGuard()
		{
			if (!shadow_verify(saved_address, function_name))
				exit(EXIT_FAILURE);
		}
};

void function_c()
{
	ShadowGuard guard("function_c", __builtin_return_address(0));
	cout << "[function_c] Running normally." << endl;
}

void function_b()
{
	ShadowGuard guard("function_b", __builtin_return_address(0)); //replacing 0 with 1 means get the addr of who called f that called f_b
	cout << "[function_b] Calling function_c..." << endl;
	function_c();
	cout << "[function_b] Returned from function_c." << endl;
}

void function_a()
{
	ShadowGuard guard("function_a", __builtin_return_address(0)); //a built-in command into the compiler that saves the return address
	cout << "[function_a] Calling function_b..." << endl;
	function_b();
	cout << "[function_a] Returned from function_b." << endl;
}

void attacker_target()
{
	cout << "[!!!] attacker_target() reached - ROP succeeded!" << endl;
}

void simulate_rop_attack()
{
	cout << "\n--- ROP Attack Simulation ---" << endl;

	void* real_address = __builtin_return_address(0);
	shadow_push(real_address, "rop_victim");

	//This is what the attacker wants to put on the stack.
	void* fake_address = reinterpret_cast<void*>(&attacker_target); //address of attacker_target function
	cout << "[ATTACK] Replacing return address with: " << fake_address << endl;

	if (!shadow_verify(fake_address, "rop_victim"))
		cout << "[RESULT] ROP attack was detected and blocked." << endl;
}

int main()
{
	cout << "==== Shadow Stack Demo ====" << endl << endl;

	cout << "--- Test Case 1: Normal Call Chain ---" << endl;
	function_a();

	cout << "\nShadow stack state: "
	     << (shadow_stack.empty() ? "empty" : "not empty")
	     << " | violations=" << violation_count << endl;

	simulate_rop_attack();

	cout << "\nTotal violations caught: " << violation_count << endl;

	return 0;
}
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <ctime>
using namespace std;

// ===============================================
// PART 1: STACK CANARY
// ===============================================

uint64_t canary_secret = 0;

void setup_canary()
{
	srand((unsigned)time(nullptr));
	canary_secret = ((uint64_t)rand() << 32) | rand();

	if (canary_secret == 0)
		canary_secret = 0xCAFEBABEDEADBEEFULL;

	cout << "[SETUP] Canary secret created: 0x" << hex << canary_secret << dec << endl;
}

class CanaryGuard
{
	private:
		string   function_name;
		uint64_t local_canary;

	public:
		CanaryGuard(const string& name)
		{
			function_name = name;
			local_canary  = canary_secret;
		}

		~CanaryGuard()
		{
			if (local_canary != canary_secret)
			{
				cout << "\n[ALERT] Canary corrupted in: " << function_name << endl;
				cout << "[ALERT] Expected : 0x" << hex << canary_secret << endl;
				cout << "[ALERT] Found    : 0x" << local_canary << dec << endl;
				exit(EXIT_FAILURE);
			}
			cout << "[CANARY-OK] " << function_name << " | canary intact" << endl;
		}

		void corrupt_canary(uint64_t fake_value)
		{
			cout << "[ATTACK] Corrupting canary in: " << function_name << endl;
			local_canary = fake_value;
		}
};

// ===============================================
// PART 2: SHADOW STACK
// ===============================================

struct ShadowEntry
{
	void*  return_address;
	string function_name;
};

vector<ShadowEntry> shadow_stack;
int violation_count = 0;

void shadow_push(void* addr, const string& name)
{
	shadow_stack.push_back({addr, name});
}

bool shadow_verify(void* actual, const string& name)
{
	if (shadow_stack.empty()) { cerr << "Shadow stack empty!\n"; exit(1); }

	void* stored = shadow_stack.back().return_address;
	shadow_stack.pop_back();

	if (stored != actual)
	{
		violation_count++;
		cout << "\n[ALERT] Return address hijack in: " << name << endl;
		cout << "[ALERT] Stored : " << stored << endl;
		cout << "[ALERT] Actual : " << actual << endl;
		return false;
	}
	cout << "[SHADOW-OK] " << name << " | return address verified" << endl;
	return true;
}

// ===============================================
// PART 3: COMBINED GUARD
// ===============================================

class ProtectionGuard
{
	private:
		CanaryGuard canary_guard;
		string      function_name;
		void*       saved_address;

	public:
		ProtectionGuard(const string& name, void* caller_addr)
			: canary_guard(name),
			  function_name(name),
			  saved_address(caller_addr)
		{
			shadow_push(saved_address, function_name);
			cout << "[PROTECTED] " << function_name << " | both layers active" << endl;
		}

		~ProtectionGuard()
		{
			if (!shadow_verify(saved_address, function_name))
				exit(EXIT_FAILURE);
		}

		void corrupt_canary(uint64_t v) { canary_guard.corrupt_canary(v); }
};

// ===============================================
// DEMO FUNCTIONS
// ===============================================

void process_data(const string& data)
{
	ProtectionGuard guard("process_data", __builtin_return_address(0));
	char buffer[64];
	strncpy(buffer, data.c_str(), 63);
	buffer[63] = '\0';
	cout << "[process_data] Received: " << buffer << endl;
}

void calculate(int a, int b)
{
	ProtectionGuard guard("calculate", __builtin_return_address(0));
	cout << "[calculate] " << a << " + " << b << " = " << (a + b) << endl;
}

void level_c()
{
	ProtectionGuard guard("level_c", __builtin_return_address(0));
	cout << "[level_c] Leaf function running." << endl;
}

void level_b()
{
	ProtectionGuard guard("level_b", __builtin_return_address(0));
	cout << "[level_b] Calling level_c..." << endl;
	level_c();
}

void level_a()
{
	ProtectionGuard guard("level_a", __builtin_return_address(0));
	cout << "[level_a] Calling level_b..." << endl;
	level_b();
}

// ===============================================
// ATTACK SIMULATIONS
// ===============================================

void simulate_canary_attack()
{
	cout << "\n--- Canary Attack Simulation ---" << endl;
	CanaryGuard guard("canary_victim");
	guard.corrupt_canary(0xBADBADBADBADBADULL);
	cout << "[canary_victim] Attempting to return..." << endl;
}

void simulate_rop_attack()
{
	cout << "\n--- ROP Attack Simulation ---" << endl;
	void* real_addr = __builtin_return_address(0);
	shadow_push(real_addr, "rop_victim");

	void* fake_addr = reinterpret_cast<void*>(0xDEADBEEFDEADBEEFULL);
	cout << "[ATTACK] Forged return address: " << fake_addr << endl;

	if (!shadow_verify(fake_addr, "rop_victim"))
		cout << "[RESULT] ROP attack detected and blocked." << endl;
}

// ===============================================
// MAIN
// ===============================================

int main()
{
	cout << "==== Combined Protection Demo ====" << endl << endl;

	setup_canary();
	cout << endl;

	cout << "--- Test Case 1: Normal Execution ---" << endl;
	process_data("Secure OS Spring 2026");
	calculate(42, 58); //demo function
	level_a();

	cout << "\n--- Test Case 2: Canary Attack ---" << endl;
	simulate_canary_attack();

	cout << "\n--- Test Case 3: ROP Attack ---" << endl;
	simulate_rop_attack();

	cout << "\nTotal shadow violations: " << violation_count << endl;
	cout << "Program finished." << endl;

	return 0;
}
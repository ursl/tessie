#include <iostream>

#include <stack>
#include <queue>

using namespace std;

// ----------------------------------------------------------------------
void stacks() {
  std::stack<int> canbus1;

  cout << "--- stacks LIFO ---" << endl;
  cout << "push 1: 101" << endl;
  canbus1.push(101);

  cout << "push 2: 102" << endl;
  canbus1.push(102);

  cout << "top 1: " << canbus1.top() << " size: " << canbus1.size() << endl;
  canbus1.pop();
  cout << "top 2: " << canbus1.top() << " size: " << canbus1.size() << endl;
  canbus1.pop();
  cout << "final size: " << canbus1.size() << endl;
}

// ----------------------------------------------------------------------
void queues() {
  std::queue<int> canbus1;

  cout << "--- queues FIFO ---" << endl;
  cout << "push 1: 101" << endl;
  canbus1.push(101);

  cout << "push 2: 102" << endl;
  canbus1.push(102);

  cout << "front 1: " << canbus1.front() << " size: " << canbus1.size() << endl;
  canbus1.pop();
  cout << "front 2: " << canbus1.front() << " size: " << canbus1.size() << endl;
  canbus1.pop();
  cout << "final size: " << canbus1.size() << endl;
}

// ----------------------------------------------------------------------
int main() {

  stacks(); 
  queues();
  
  return 0;
}

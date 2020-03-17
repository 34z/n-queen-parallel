#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
using namespace std;


class MyTimer {
private:
	std::chrono::_V2::system_clock::time_point m_start;
	std::chrono::_V2::system_clock::time_point m_end;
public:
	void start() { m_start = std::chrono::_V2::system_clock::now(); }
	void end() { m_end = std::chrono::_V2::system_clock::now(); }
	double get_duration() {
		return (double)std::chrono::duration_cast<std::chrono::microseconds>(m_end - m_start).count()
			* std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
	}
};

class Queen {
private:
	int size;
	vector<int> board;
public:
	Queen(int s): size(s) {}
	bool is_solved() { return board.size() == size; }
	vector<Queen*>* solve(int par=1) {
		if (par > 1) {
			return solve_par(par);
		}
		auto ret = new vector<Queen*>;
		ret->push_back(this);
		for (int i = 0; i < this->size; i++) {
			auto t = new vector<Queen*>;
			for (auto &q : *ret) {
				auto s = q->solve_once();
				t->insert(t->end(), s->begin(), s->end());
				if (q != this) {
					delete q;
				}
			}
			delete ret;
			ret = t;
		}
		return ret;
	}
	vector<Queen*>* solve_par(int par) {
		auto ret = new vector<Queen*>;
		auto pool = new vector<Queen*>;
		pool->push_back(this);
		mutex mtx_pool, mtx_ret;
		auto f = [=, &mtx_pool, &mtx_ret]{
			mtx_pool.lock();
			// cout << "thread " << this_thread::get_id() << endl;
			bool empty = pool->empty();
			mtx_pool.unlock();

			while (!empty) {
				mtx_pool.lock();
				// cout << pool->size() << endl;
				Queen *q = (*pool)[pool->size()-1];
				pool->pop_back();
				mtx_pool.unlock();

				auto l = q->solve_once();

				if (!l->empty() && (*l)[0]->is_solved()) {
					mtx_ret.lock();
					ret->insert(ret->end(), l->begin(), l->end());
					mtx_ret.unlock();
				} else {
					mtx_pool.lock();
					pool->insert(pool->end(), l->begin(), l->end());
					mtx_pool.unlock();
				}
				if (q != this) delete q;
				delete l;
				mtx_pool.lock();
				empty = pool->empty();
				mtx_pool.unlock();
			}
		};
		
		vector<thread> ths;
		for (int i = 0; i < par; i++) {
			ths.push_back(thread(f));
			// cout << ths.size() << endl;
		}
		for (int i = 0; i < par; i++) {
			ths[i].join();
		}
		delete pool;
		return ret;
	}
	vector<Queen*>* solve_once() {
		auto ret = new vector<Queen*>;
		if (is_solved()) {
			ret->push_back(this);
		} else {
			int next_row = board.size();
			for (int i = 0; i < size; i++) {
				Queen *q = new Queen(*this);
				q->board.push_back(i);
				if (q->check_last_line()) {
					ret->push_back(q);
				} else {
					delete q;
				}
			}
		}
		return ret;
	}
	bool check_last_line() const {
		bool ret = true;
		int filled = board.size();
		for (int i = 0; i < filled-1; i++) {
			int row_diff = filled - i - 1;
			int col_diff = abs(board[i] - board[filled-1]);
			if (col_diff == 0 || col_diff == row_diff) {
				ret = false;
				break;
			}
		}
		return ret;
	}
	void draw() {
		cout << " ";
		for (int i = 0; i < size; i++) {
			cout << " " << i + 1;
		}
		cout << endl;
		for (int i = 0; i < size; i++) {
			cout << i + 1;
			for (int j = 0; j < size; j++) {
				cout << " ";
				if (i < board.size() && j == board[i]) {
					cout << "x";
				} else {
					cout << " ";
				}
			}
			cout << endl;
		}
	}

};

void test(int size);

int main(int argc, char const *argv[])
{
	// for (int i = 8; i < 12; i++) {
	// 	cout << "===============" << endl;
	// 	cout << "Testing " << i << " queens" << endl;
	// 	cout << "=======" << endl;

	// 	test(i);

	// 	cout << "===============" << endl;
	// }
	test(13);

	return 0;
}

void test(int size)
{
	Queen q(size);
	MyTimer t;

	for (int par = 2; par < 6; par++) {
		cout << "=====> parallel " << par << endl;
		t.start();
		auto l = q.solve(par);
		t.end();
		cout << "total " << l->size() << endl;
		cout << t.get_duration() << endl;
		delete l;
	}
	
	cout << "=====> serial" << endl;
	t.start();
	auto l = q.solve();
	t.end();
	cout << "total " << l->size() << endl;
	cout << t.get_duration() << endl;
	delete l;

	// int par = 4;
}
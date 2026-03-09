#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

enum class Status { success, failure };

class Person {
 public:
  Person(std::string name, int grade, int salary, int id,
         bool throw_name = false, bool throw_grade = false,
         bool throw_salary = false, bool throw_id = false)
      : name_(std::move(name)),
        grade_(grade),
        salary_(salary),
        id_(id),
        throw_name_(throw_name),
        throw_grade_(throw_grade),
        throw_salary_(throw_salary),
        throw_id_(throw_id) {}

  const std::string& name() const {
    if (throw_name_) {
      throw std::runtime_error("name() failed");
    }
    return name_;
  }

  int grade() const {
    if (throw_grade_) {
      throw std::runtime_error("grade() failed");
    }
    return grade_;
  }

  int salary() const {
    if (throw_salary_) {
      throw std::runtime_error("salary() failed");
    }
    return salary_;
  }

  int id() const {
    if (throw_id_) {
      throw std::runtime_error("id() failed");
    }
    return id_;
  }

 private:
  std::string name_;
  int grade_;
  int salary_;
  int id_;
  bool throw_name_;
  bool throw_grade_;
  bool throw_salary_;
  bool throw_id_;
};

void save(Status status, int id) {
  if (id < 0) {
    throw std::runtime_error("save() failed");
  }
  std::cout << "save: " << (status == Status::success ? "success" : "failure")
            << ", id=" << id << '\n';
}

void test(const Person& person) {
  std::cout << "test : " << person.name() << '\n';

  if (person.grade() == 10 || person.salary() > 1'000'000) {
    save(Status::success, person.id());
  } else {
    save(Status::failure, person.id());
  }
}

int main() {
  std::cout << "Potential throw points in test(Person const&):\n";
  std::cout << "1) operator<< for std::cout while printing prefix or name\n";
  std::cout << "2) person.name()\n";
  std::cout << "3) person.grade()\n";
  std::cout
      << "4) person.salary() (only if grade() != 10 due to short-circuit)\n";
  std::cout << "5) person.id() in both branches\n";
  std::cout << "6) save(Status, id) in both branches\n";

  std::cout << "Normal branching point:\n";
  std::cout
      << "- condition (person.grade() == 10 || person.salary() > 1'000'000)\n";

  try {
    Person ok{"Alice", 10, 100, 1};
    test(ok);

    Person rich{"Bob", 7, 2'000'000, 2};
    test(rich);

    Person failing_name{"Eve", 5, 100, 3, true};
    test(failing_name);
  } catch (const std::exception& exception) {
    std::cerr << "Caught exception: " << exception.what() << '\n';
  }

  return 0;
}

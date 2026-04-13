#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

namespace builder_example {

struct Entity {
  int x = 0;
  int y = 0;
};

class Builder {
 public:
  virtual ~Builder() = default;

  std::unique_ptr<Entity> make_entity() {
    entity_ = std::make_unique<Entity>();
    set_x();
    set_y();
    return std::move(entity_);
  }

  virtual void set_x() const = 0;
  virtual void set_y() const = 0;

 protected:
  mutable std::unique_ptr<Entity> entity_;
};

class BuilderClient final : public Builder {
 public:
  void set_x() const override { entity_->x = 1; }
  void set_y() const override { entity_->y = 1; }
};

}  // namespace builder_example

namespace factory_example {

class Entity {
 public:
  virtual ~Entity() = default;
  virtual int kind() const = 0;
};

class Client final : public Entity {
 public:
  int kind() const override { return 1; }
};

class Server final : public Entity {
 public:
  int kind() const override { return 2; }
};

class Factory {
 public:
  virtual ~Factory() = default;
  virtual std::unique_ptr<Entity> make_entity() const = 0;
};

class FactoryClient final : public Factory {
 public:
  std::unique_ptr<Entity> make_entity() const override {
    return std::make_unique<Client>();
  }
};

}  // namespace factory_example

namespace prototype_example {

class Entity {
 public:
  virtual ~Entity() = default;
  virtual std::unique_ptr<Entity> copy() const = 0;
  virtual int kind() const = 0;
};

class Client final : public Entity {
 public:
  std::unique_ptr<Entity> copy() const override {
    return std::make_unique<Client>(*this);
  }
  int kind() const override { return 1; }
};

class Server final : public Entity {
 public:
  std::unique_ptr<Entity> copy() const override {
    return std::make_unique<Server>(*this);
  }
  int kind() const override { return 2; }
};

class Prototype {
 public:
  Prototype() {
    entities_.push_back(std::make_unique<Client>());
    entities_.push_back(std::make_unique<Server>());
  }

  std::unique_ptr<Entity> make_client() const { return entities_.at(0)->copy(); }
  std::unique_ptr<Entity> make_server() const { return entities_.at(1)->copy(); }

 private:
  std::vector<std::unique_ptr<Entity>> entities_;
};

}  // namespace prototype_example

namespace composite_example {

class Entity {
 public:
  virtual ~Entity() = default;
  virtual int test() const = 0;
};

class Client final : public Entity {
 public:
  int test() const override { return 1; }
};

class Server final : public Entity {
 public:
  int test() const override { return 2; }
};

class Composite final : public Entity {
 public:
  void add(std::unique_ptr<Entity> entity) { entities_.push_back(std::move(entity)); }

  int test() const override {
    int value = 0;
    for (const auto& entity : entities_) {
      if (entity) {
        value += entity->test();
      }
    }
    return value;
  }

 private:
  std::vector<std::unique_ptr<Entity>> entities_;
};

std::unique_ptr<Entity> make_composite(std::size_t clients, std::size_t servers) {
  auto composite = std::make_unique<Composite>();
  for (std::size_t i = 0; i < clients; ++i) {
    composite->add(std::make_unique<Client>());
  }
  for (std::size_t i = 0; i < servers; ++i) {
    composite->add(std::make_unique<Server>());
  }
  return composite;
}

}  // namespace composite_example

namespace observer_example {

class Observer {
 public:
  virtual ~Observer() = default;
  virtual void test(int value) const = 0;
};

class Entity {
 public:
  void add(std::unique_ptr<Observer> observer) {
    observers_.push_back(std::move(observer));
  }

  void set(int value) {
    value_ = value;
    notify_all();
  }

 private:
  void notify_all() const {
    for (const auto& observer : observers_) {
      if (observer) {
        observer->test(value_);
      }
    }
  }

  int value_ = 0;
  std::vector<std::unique_ptr<Observer>> observers_;
};

class Client final : public Observer {
 public:
  void test(int value) const override {
    std::cout << "Client::test : x = " << value << '\n';
  }
};

class Server final : public Observer {
 public:
  void test(int value) const override {
    std::cout << "Server::test : x = " << value << '\n';
  }
};

}  // namespace observer_example

int main() {
  {
    auto builder = std::make_unique<builder_example::BuilderClient>();
    auto entity = builder->make_entity();
    assert(entity->x == 1);
    assert(entity->y == 1);
  }

  {
    std::unique_ptr<factory_example::Factory> factory =
        std::make_unique<factory_example::FactoryClient>();
    auto entity = factory->make_entity();
    assert(entity->kind() == 1);
  }

  {
    prototype_example::Prototype prototype;
    auto client = prototype.make_client();
    auto server = prototype.make_server();
    assert(client->kind() == 1);
    assert(server->kind() == 2);
  }

  {
    auto composite = std::make_unique<composite_example::Composite>();
    for (std::size_t i = 0; i < 5; ++i) {
      composite->add(composite_example::make_composite(1, 1));
    }
    std::unique_ptr<composite_example::Entity> entity = std::move(composite);
    assert(entity->test() == 15);
  }

  {
    observer_example::Entity entity;
    entity.add(std::make_unique<observer_example::Client>());
    entity.add(std::make_unique<observer_example::Server>());
    entity.set(1);
    entity.set(2);
  }

  return 0;
}

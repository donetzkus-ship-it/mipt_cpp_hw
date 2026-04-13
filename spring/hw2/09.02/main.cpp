#include <cassert>
#include <iostream>
#include <memory>
#include <queue>
#include <stack>

class Tree {
 public:
  struct Node {
    explicit Node(int value) : value(value) {
      std::cout << "Node created: " << value << '\n';
    }

    ~Node() { std::cout << "Node destroyed: " << value << '\n'; }

    int value = 0;
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;
    std::weak_ptr<Node> parent;
  };

  void traverse_v1() const {
    if (!root) {
      return;
    }

    std::queue<std::shared_ptr<Node>> nodes;
    nodes.push(root);

    while (!nodes.empty()) {
      const std::shared_ptr<Node> current = nodes.front();
      nodes.pop();

      std::cout << current->value << ' ';

      if (current->left) {
        nodes.push(current->left);
      }
      if (current->right) {
        nodes.push(current->right);
      }
    }

    std::cout << '\n';
  }

  void traverse_v2() const {
    if (!root) {
      return;
    }

    std::stack<std::shared_ptr<Node>> nodes;
    nodes.push(root);

    while (!nodes.empty()) {
      const std::shared_ptr<Node> current = nodes.top();
      nodes.pop();

      std::cout << current->value << ' ';

      if (current->right) {
        nodes.push(current->right);
      }
      if (current->left) {
        nodes.push(current->left);
      }
    }

    std::cout << '\n';
  }

  std::shared_ptr<Node> root;
};

int main() {
  Tree tree;

  tree.root = std::make_shared<Tree::Node>(1);
  tree.root->left = std::make_shared<Tree::Node>(2);
  tree.root->right = std::make_shared<Tree::Node>(3);
  tree.root->left->left = std::make_shared<Tree::Node>(4);
  tree.root->left->right = std::make_shared<Tree::Node>(5);
  tree.root->right->left = std::make_shared<Tree::Node>(6);
  tree.root->right->right = std::make_shared<Tree::Node>(7);

  tree.root->left->parent = tree.root;
  tree.root->right->parent = tree.root;
  tree.root->left->left->parent = tree.root->left;
  tree.root->left->right->parent = tree.root->left;
  tree.root->right->left->parent = tree.root->right;
  tree.root->right->right->parent = tree.root->right;

  assert(!tree.root->left->parent.expired());
  assert(tree.root->left->parent.lock() == tree.root);

  std::cout << "BFS: ";
  tree.traverse_v1();

  std::cout << "DFS: ";
  tree.traverse_v2();

  tree.root.reset();
  std::cout << "Tree destroyed\n";

  return 0;
}

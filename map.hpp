/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"
#include<map>


template<class T>
class linkStack
{
private:
	struct Stacknode
	{
		T data;
		Stacknode* next;
		Stacknode(const T&d,Stacknode* n=NULL):data(d),next(n){}
	};

	Stacknode* top;
public:

	linkStack():top(NULL){}
	~linkStack()
	{
		Stacknode* tmp = top;
		Stacknode* del;
		while (tmp)
		{
			del = tmp;
			tmp = tmp->next;
			delete del;
		}
	}

	bool isEmpty()
	{
		return top == NULL;
	}

	void push(const T&x)
	{
		Stacknode* n = new Stacknode(x,top);
		top =n;
	}

	T pop()
	{
		T record = top->data;
		Stacknode* del = top;
		top = top->next;
		delete del;
		return record;
	}
};


enum COLOR
{
	RED,
	BLACK
};

namespace sjtu {

template<
	class Key,
	class T,
	class Compare = std::less<Key>
> class map {

public:
	class iterator;
	typedef pair<const Key, T> value_type;
private:
	struct node
	{

		value_type* data;
		node* left;
		node* right;
		node* dad;
		int color;

		node(const value_type&element, node* lt = NULL, node* rt = NULL, int h = RED,node* d=NULL)
			:left(lt), right(rt), color(h) ,dad(d)
		{
			data = new value_type(element);
		}

	};
	

	inline bool is_equal(const Key&x, const Key&y) const
	{
		if (Compare()(x, y) || Compare()(y, x)) return false;
		return true;
	}
	node* root;
	size_t sz;

	node* getSite(const Key&x) const
	{
		node* r = root;
		while (r&&!is_equal(r->data->first , x))
		{
			if (Compare()(x,r->data->first))
				r = r->left;
			else r = r->right;
		}
		return r;
	}

	void makeEmpty(node* &r)
	{
		if (r != NULL)
		{
			makeEmpty(r->left);
			makeEmpty(r->right);
			delete r;
		}
		r = NULL;
	}

	void LL(node* &r)
	{
		node* r1 = r->left;
		r->left = r1->right;
		if(r1->right)
			r1->right->dad= r;
		r1->dad = r->dad;
		r->dad = r1;
		r1->right = r;
		r = r1;
	}

	void RR(node* &r)
	{
		node* r1 = r->right;
		r -> right = r1->left;
		if(r1->left)
			r1->left->dad= r;
		r1->dad = r->dad;
		r->dad = r1;
		r1->left = r;
		r = r1;
	}

	void LR(node* &r)
	{
		RR(r->left);
		LL(r);
	}

	void RL(node* &r)
	{
		LL(r->right);
		RR(r);
	}

	void reLink(node* oldp, node* newp, linkStack<node*> &path)
	{
		if (path.isEmpty()) root = newp;
		else
		{
			node* grandParent = path.pop();
			if (grandParent->left == oldp)
			{
				grandParent->left = newp;
				if(newp) newp->dad = grandParent;
			}
			else
			{
				
				grandParent->right = newp;
				if(newp) newp->dad = grandParent;
			}
			path.push(grandParent);
		}
	}


	void insertReBalance(node* r, linkStack<node*> &path)
	{
		Compare com;
		node* parent, *grandParent, *uncle, *rootOfsubTree;
		parent = path.pop();

		while (parent->color == RED)
		{
			if (parent == root) {
				parent->color = BLACK;
				return;
			}

			grandParent = rootOfsubTree = path.pop();
			if (com(parent->data->first, grandParent->data->first))
				uncle = grandParent->right;
			else
				uncle = grandParent->left;

			if (uncle == NULL || uncle->color == BLACK)
			{
				if (grandParent->left == parent)
				{
					if (r == parent->left)
					{
						parent->color = BLACK;
						grandParent->color = RED;
						LL(grandParent);
					}
					else
					{
						grandParent->color = RED;
						r->color = BLACK;
						LR(grandParent);
					}
				}

				else if (r == parent->right)
				{
					parent->color = BLACK;
					grandParent->color = RED;
					RR(grandParent);
				}

				else
				{
					grandParent->color = RED;
					r->color = BLACK;
					RL(grandParent);
				}
				reLink(rootOfsubTree, grandParent, path);
				return;
			}

			else
			{
				grandParent->color = RED;
				parent->color = BLACK;
				uncle->color = BLACK;
				if (root == grandParent) {
					root->color = BLACK;
					return;
				}

				r = grandParent;
				parent = path.pop();
			}
		}
	}

	pair<node*, bool> _insert(const value_type&x)
	{
		Compare com;
		linkStack<node*> path;
		node *r, *parent;
		if (root == NULL)
		{
			root = new node(x, NULL, NULL, BLACK);
			return pair<node*, bool>(root, true);
		}

		r = root;
		while (r&&!is_equal(r->data->first , x.first))
		{
			path.push(r);
			if (com(r->data->first , x.first)) r = r -> right;
			else r = r->left;
		}

		if (r) return pair<node*, bool>(r, false);
		r = new node(x, NULL, NULL);

		parent = path.pop();
		if (com(x.first , parent->data->first)) {
			parent->left = r;
			if(r) r->dad = parent;
		}
		else {
			parent->right = r;
			if(r) r->dad = parent;
		}

		if (parent->color == BLACK) return pair<node*, bool>(r, true);

		path.push(parent);
		insertReBalance(r, path);
		return pair<node*, bool>(r, true);
	}

	void remove(const Key&x)
	{
		Compare com;
		linkStack<node*> path;
		node* r = root, *old, *parent = NULL;

		while (r && !is_equal(r->data->first, x))
		{
			path.push(r);
			if (com(x, r->data->first))  r = r->left;
			else r = r->right;
		}

		if (r == NULL) return;
		if (r->left&&r->right)
		{
			path.push(r);
			old = r;
			r = r->right;
			while (r->left) {
				path.push(r);
				r = r->left;
			}
			old->data = r->data;
		}

		sz--;
		if (r == root)
		{
			root = (r->left ? r->left : r->right);
			if (root) root->color = BLACK;
			return;
		}
		
		parent = path.pop();
		old = r;
		r = (r->left ? r->left : r->right);
		if (parent->left == old)
		{
			parent->left = r;
			if(r) r->dad = parent;
		}

		else
		{
			parent->right = r;
			if(r) r->dad = parent;
		}

		if (old->color == RED)
		{
			delete old;
			return;
		}
		delete old;
		old = NULL;

		if (r)
		{
			r->color = BLACK;
			return;
		}

		path.push(parent);
		removeRebalance(r, path);
	}

	void removeRebalance(node*r, linkStack<node*> &path)
	{
		node* parent, *sibling = NULL, *rootOfSubTree;
		parent = rootOfSubTree = path.pop();
		while (parent)
		{
			if (parent->left == r) sibling = parent->right;
			else sibling = parent->left;


			if (sibling == NULL)
			{
				if (parent->color == RED) {
					parent->color = BLACK;
					return;
				}
				else {
					r = parent;
					if (r == root) return;
					else {
						parent = rootOfSubTree = path.pop();
					}
				}
			}
			else
			{
				if (sibling->color == RED)
				{
					sibling->color = BLACK;
					parent->color = RED;
					if (parent->left == r)
						RR(parent);
					else LL(parent);

					reLink(rootOfSubTree, parent, path);
					path.push(parent);
					parent = rootOfSubTree;
				}

				else
				{
					if ((sibling->left == NULL || sibling->left->color == BLACK) &&
						(sibling->right == NULL || sibling->right->color == BLACK))
					{
						sibling->color = RED;
						if (parent->color == RED)
						{
							parent->color = BLACK;
							return;
						}

						else
						{
							r = parent;
							if (r == root) return;
							else parent = rootOfSubTree = path.pop();
						}
					}
					else break;
				}
			}
		}

		if (parent->left == r)
		{
			if (sibling->left&&sibling->left->color == RED)
			{
				sibling->left->color = parent->color;
				parent->color = BLACK;
				RL(parent);
				reLink(rootOfSubTree, parent, path);
			}

			else
			{
				sibling->color = parent->color;
				sibling->right->color = BLACK;
				parent->color = BLACK;
				RR(parent);
				reLink(rootOfSubTree, parent, path);
			}
		}

		else
		{
			if (sibling->right&&sibling->right->color == RED)
			{
				sibling->right->color = parent->color;
				parent->color = BLACK;
				LR(parent);
				reLink(rootOfSubTree, parent, path);
			}

			else
			{
				sibling->color = parent->color;
				sibling->left->color = BLACK;
				parent->color = BLACK;
				LL(parent);
				reLink(rootOfSubTree, parent, path);
			}
		}
	}

public:
	/**
	 * the internal type of data.
	 * it should have a default constructor, a copy constructor.
	 * You can use sjtu::map as value_type by typedef.
	 */


	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = map.begin(); --it;
	 *       or it = map.end(); ++end();
	 */
	class const_iterator;
	struct node;
	class iterator {
		/**
		 * TODO add data members
		 *   just add whatever you want.
		 */


	public:
		node *current;
		map* owner;
		bool is_dummy;


		iterator(node *n = NULL, map* o = NULL,bool bo=false)
			:current(n)
			, owner(o)
			,is_dummy(bo){}

		~iterator() {}

		iterator(const iterator &other) {
			current = other.current;
			owner = other.owner;
			is_dummy = other.is_dummy;
		}

		iterator &operator=(const iterator&other)
		{
			current = other.current;
			owner = other.owner;
			is_dummy = other.is_dummy;
			return *this;
		}
		/**
		 * return a new iterator which pointer n-next elements
		 *   even if there are not enough elements, just return the answer.
		 * as well as operator-
		 */
		 /**
		  * TODO iter++
		  */
		iterator operator++(int) 
		{
			if (is_dummy) throw invalid_iterator();
			iterator ans = *this;
			if (current->right)
			{
				current = current->right;
				while (current->left) current = current->left;
				return ans;
			}

			node* record = current;
			while(current->dad&&current == current->dad->right)
			{
				current = current->dad;
			}

			if(current->dad) current = current->dad;
			else
			{
				is_dummy = true;
				//此时变成end()
			}
			return ans;

		}
		/**
		 * TODO ++iter
		 */
		iterator & operator++() 
		{
			if (is_dummy) throw invalid_iterator();
			if (current->right)
			{
				current = current->right;
				while (current->left) current = current->left;
				return *this;
			}

			node* record = current;
			while (current->dad&&current == current->dad->right)
			{
				current = current->dad;
			}

			if (current->dad) current = current->dad;
			else
			{
				is_dummy = true;
				//此时变成end()
			}

			return *this;
		}

		iterator operator--(int) 
		{
			iterator ans = *this;
			if (is_dummy)
			{
				is_dummy = false;
				node* tmp = owner->root;
				if (tmp==NULL) throw invalid_iterator();
				while(tmp->right) tmp = tmp->right;
				current = tmp;
				return ans;
			}
			
			if (current->left)
			{
				current = current->left;
				while (current->right) current = current->right;
				return ans;
			}

			while (current->dad&&current == current->dad->left)
			{
				current = current->dad;
			}

			if (current->dad) current = current->dad;
			else throw invalid_iterator();
			return ans;
		}
		/**
		 * TODO --iter
		 */
		iterator & operator--()
		{
			if (is_dummy)
			{
				is_dummy = false;
				node* tmp = owner->root;
				if (tmp==NULL) throw invalid_iterator();
				while (tmp->right) tmp = tmp->right;
				current = tmp;
				return *this;
			}

			if (current->left)
			{
				current = current->left;
				while (current->right) current = current->right;
				return *this;
			}

			while (current->dad&&current == current->dad->left)
			{
				current = current->dad;
			}

			if (current->dad) current = current->dad;
			else throw invalid_iterator();

			return *this;
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const 
		{
			if(is_dummy) throw invalid_iterator();
			return *current->data;
		}
		bool operator==(const iterator &rhs) const 
		{
			if (owner != rhs.owner) return false;
			if (is_dummy) return rhs.is_dummy;
			return current == rhs.current&&is_dummy==rhs.is_dummy;
		}
		bool operator==(const const_iterator &rhs) const 
		{
			if (owner != rhs.owner) return false;
			if (is_dummy) return rhs.is_dummy;
			return current == rhs.current&&is_dummy == rhs.is_dummy;
		}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const 
		{
			if (owner != rhs.owner) return true;
			return !(*this==rhs);
		}
		bool operator!=(const const_iterator &rhs) const
		{
			if (owner != rhs.owner) return true;
			return !(*this == rhs);
		}

		/**
		 * for the support of it->first.
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		value_type* operator->() const noexcept 
		{
			return current->data;
		}
	};
	class const_iterator {
		// it should has similar member method as iterator.
		//  and it should be able to construct from an iterator.
	private:
		// data members.
	public:
		node *current;
		const map* owner;
		bool is_dummy;

		const_iterator(node* c=NULL, const map* o=NULL,bool bo=false) :current(c), owner(o),is_dummy(bo) {}

		const_iterator(const const_iterator &other)
		{
			// TODO
			current = other.current;
			owner = other.owner;
			is_dummy = other.is_dummy;
		}
		const_iterator(const iterator &other) 
		{
			// TODO
			current = other.current;
			owner = other.owner;
			is_dummy = other.is_dummy;
		}

		const_iterator operator++(int)
		{
			if (is_dummy) throw invalid_iterator();
			const_iterator ans = *this;
			if (current->right)
			{
				current = current->right;
				while (current->left) current = current->left;
				return ans;
			}

			node* record = current;
			while (current->dad&&current == current->dad->right)
			{
				current = current->dad;
			}

			if (current->dad) current = current->dad;
			else
			{
				is_dummy = true;
				//此时变成end()
			}
			return ans;

		}
		/**
		* TODO ++iter
		*/
		const_iterator & operator++()
		{
			if (is_dummy) throw invalid_iterator();
			if (current->right)
			{
				current = current->right;
				while (current->left) current = current->left;
				return *this;
			}

			node* record = current;
			while (current->dad&&current == current->dad->right)
			{
				current = current->dad;
			}

			if (current->dad) current = current->dad;
			else
			{
				is_dummy = true;
				//此时变成end()
			}

			return *this;
		}

		const_iterator operator--(int)
		{
			const_iterator ans = *this;
			if (is_dummy)
			{
				is_dummy = false;
				node* tmp = owner->root;
				if (tmp==NULL) throw invalid_iterator();
				while (tmp->right) tmp = tmp->right;
				current = tmp;
				return ans;
			}

			if (current->left)
			{
				current = current->left;
				while (current->right) current = current->right;
				return ans;
			}

			while (current->dad&&current == current->dad->left)
			{
				current = current->dad;
			}

			if (current->dad) current = current->dad;
			else throw invalid_iterator();
			return ans;
		}
		/**
		* TODO --iter
		*/
		const_iterator & operator--()
		{
			if (is_dummy)
			{
				is_dummy = false;
				node* tmp = owner->root;
				if (tmp == NULL) throw invalid_iterator();
				while (tmp->right) tmp = tmp->right;
				current = tmp;
				return *this;
			}

			if (current->left)
			{
				current = current->left;
				while (current->right) current = current->right;
				return *this;
			}

			while (current->dad&&current == current->dad->left)
			{
				current = current->dad;
			}

			if (current->dad) current = current->dad;
			else throw invalid_iterator();

			return *this;
		}
		/**
		* a operator to check whether two iterators are same (pointing to the same memory).
		*/
		value_type & operator*() const
		{
			if (is_dummy) throw invalid_iterator();
			return *current->data;
		}
		bool operator==(const iterator &rhs) const
		{
			if (owner != rhs.owner) return false;
			if (is_dummy) return rhs.is_dummy;
			return current == rhs.current&&is_dummy == rhs.is_dummy;
		}
		bool operator==(const const_iterator &rhs) const
		{
			if (owner != rhs.owner) return false;
			if (is_dummy) return rhs.is_dummy;
			return current == rhs.current&&is_dummy == rhs.is_dummy;
		}
		/**
		* some other operator for iterator.
		*/
		bool operator!=(const iterator &rhs) const
		{
			if (owner != rhs.owner) return true;
			return !(*this == rhs);
		}
		bool operator!=(const const_iterator &rhs) const
		{
			if (owner != rhs.owner) return true;
			return !(*this == rhs);
		}

		/**
		* for the support of it->first.
		* See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		*/
		value_type* operator->() const noexcept
		{
			return current->data;
		}
		// And other methods in iterator.
		// And other methods in iterator.
		// And other methods in iterator.
	};
	/**
	 * TODO two constructors
	 */

	void deep_copy(node* &r, node* other_r,node* tmp)
	{
		if (other.r == NULL) return;
		r = new node(*other_r->data, NULL, NULL, other_r->color, tmp);
		if(other_r->left) deep_copy(r->left, other_r->left, r);
		if(other_r->right) deep_copy(r->right, other_r->right, r);
	}


public:
	map():root(NULL),sz(0){}

	map(const map &other)
	{
		if (other.root == NULL) 
		{
			root=NULL;
			return;
		}
		deep_copy(root,other.root,NULL);
		sz = other.sz;
	}
	/**
	 * TODO assignment operator
	 */
	map & operator=(const map &other)
	{
		if (root == other.root)
			return *this;
		makeEmpty(root);
		deep_copy(root, other.root, NULL);
		sz = other.sz;

		return *this;
	}
	/**
	 * TODO Destructors
	 */
	~map() { makeEmpty(root); }
	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `index_out_of_bound'
	 */
	T & at(const Key &key) 
	{
		node* tmp=getSite(key);
		if (tmp == NULL) throw index_out_of_bound();
		else return tmp->data->second;
	}
	const T & at(const Key &key) const 
	{
		node* tmp = getSite(key);
		if (tmp == NULL) throw index_out_of_bound();
		else return tmp->data->second;
	}
	/**
	 * TODO
	 * access specified element 
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) 
	{
		pair<node*, bool> insert_result = _insert(pair<const Key, T>(key, T()));
		if (insert_result.second) sz++;
		return insert_result.first->data->second;
	}
	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const 
	{
		node* tmp = getSite(key);
		if (tmp == NULL) throw index_out_of_bound();
		else return tmp->data->second;
	}
	/**
	 * return a iterator to the beginning
	 */
	iterator begin()
	{
		node* tmp = root;
		if(!tmp) return iterator(NULL, this, true);
		else
		{
			while (tmp->left) tmp = tmp->left;
			return iterator(tmp, this, false);
		}
	}
	const_iterator cbegin() const 
	{
		node* tmp = root;
		if (!tmp) return const_iterator(tmp, this, true);
		else
		{
			while (tmp->left) tmp = tmp->left;
			return const_iterator(tmp, this, false);
		}
	}
	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() 
	{
		return iterator(NULL, this, true);
	}
	const_iterator cend() const 
	{
		return const_iterator(NULL, this, true);
	}
	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const 
	{
		return root == NULL;
	}
	/**
	 * returns the number of elements.
	 */
	size_t size() const
	{
		return sz;
	}

	/**
	 * clears the contents
	 */
	void clear() 
	{
		makeEmpty(root);
		sz = 0;
	}

	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion), 
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) 
	{
		pair<node*, bool> insert_result = _insert(value);
		iterator iter(insert_result.first, this, false);
		bool bo = insert_result.second;
		if (bo) sz++;
		return pair<iterator, bool>(iter, bo);
	}
	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) 
	{
		if (pos.is_dummy||pos.owner!=this)
		{
			throw invalid_iterator();
		}
		remove(pos.current->data->first);
	}
	/**
	 * Returns the number of elements with key 
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0 
	 *     since this container does not allow duplicates.
	 * The default method of check the equivalence is !(a < b || b > a)
	 */

	size_t count(const Key &key) const
	{
		node* tmp=getSite(key);
		if (tmp) return 1;
		else return 0;
	}
	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) 
	{
		node * tmp = getSite(key);
		if (tmp) return iterator(tmp, this, false);
		else return iterator(root, this, true);
	}
	const_iterator find(const Key &key) const 
	{
		node * tmp = getSite(key);
		if (tmp) return const_iterator(tmp, this, false);
		else return const_iterator(root, this, true);
	}
};

}

#endif

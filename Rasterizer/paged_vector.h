

#pragma once
//try to avoid the iterator include as that one includes too much
#if  defined _MSC_VER
#include <xutility>
#else
#include <iterator>
#endif

#include <type_traits>

template<typename T>
class paged_vector {
	using value_type = T;
	using size_type = size_t;
	using reference = value_type&;
	using pointer = value_type*;
	using const_reference = const value_type&;
	static constexpr int bits = 6;
	static constexpr int page_size = 1 << bits;
	static constexpr size_t mask = (page_size - 1);
	//using page = void*;
	struct page {
		std::aligned_storage_t <sizeof(T), alignof(T)> elements[page_size];
	};
public:

	paged_vector() noexcept = default;
	paged_vector(const paged_vector& other) noexcept
	{
		//make sure to destroy everything
		clear();
		resize(other.last_index);

		for (int i = 0; i < other.last_index; i++) {
			operator[](i) = other[i];
		}
	}
	paged_vector& operator=(const paged_vector& other)
	{
		//make sure to destroy everything
		clear();
		resize(other.last_index);

		for (int i = 0; i < other.last_index; i++) {
			operator[](i) = other[i];
		}

		return *this;
	};

	~paged_vector() noexcept {
		resize(0);
	}
	paged_vector(paged_vector&& other) noexcept {
		resize(0);

		last_index = other.last_index;
		page_array = other.page_array;
		page_capacity = other.page_capacity;

		other.page_array = nullptr;
		other.last_index = 0;
		other.page_capacity = 0;
	}
	paged_vector& operator=(paged_vector&& other) {
		resize(0);

		last_index = other.last_index;
		page_array = other.page_array;
		page_capacity = other.page_capacity;

		other.page_array = nullptr;
		other.last_index = 0;
		other.page_capacity = 0;

		return *this;
	}

	class iterator
	{
	public:
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = int;
		using value_type = T;
		using reference = value_type&;
		using pointer = value_type*;

		iterator(int idx, paged_vector<T>* owner) : _idx(idx), _owner(owner) { }

		iterator& operator++() {
			return ++_idx, * this;
		}

		iterator operator++(int) {
			iterator orig = *this;
			return operator++(), orig;
		}

		iterator& operator+=(const difference_type value) {
			_idx += value;
			return *this;
		}

		iterator operator+(const difference_type value) const {
			iterator copy = *this;
			return (copy += value);
		}

		iterator& operator--() {
			return --_idx, * this;
		}

		iterator operator--(int) {
			iterator orig = *this;
			return operator--(), orig;
		}

		iterator& operator-=(const difference_type value) {
			_idx -= value;
			return *this;
		}

		iterator operator-(const difference_type value) const {
			iterator copy = *this;
			return (copy -= value);
		}

		difference_type operator-(const iterator& rhs) const {
			return difference_type(_idx - rhs._idx);
		}

		reference operator[](const difference_type value) const {
			int index = _idx + value;
			return (*_owner)[_idx];
		}

		reference operator*() { return (*_owner)[_idx]; }
		pointer operator->() { return _owner.at_index(_idx); }
		bool operator==(const iterator& rhs) const { return _idx == rhs._idx && _owner == rhs._owner; }
		bool operator!=(const iterator& rhs) const { return !(*this == rhs); }
		bool operator<(const iterator& rhs) const { return _idx < rhs._idx; }
		bool operator>(const iterator& rhs)const { return _idx > rhs._idx; }
		bool operator<=(const iterator& rhs) const { return !(*this > rhs); }
		bool operator>=(const iterator& rhs) const { return !(*this < rhs); }


	private:
		int _idx;
		paged_vector* _owner;
	};

	iterator begin() {
		return iterator(0, this);
	}
	iterator end() {
		return iterator(static_cast<int>(last_index), this);
	}

	reference operator[](size_type index) {
		pointer p = at_index(index);
		return *p;//reinterpret_cast<reference>(*p);
	}
	const_reference operator[](size_type index) const {
		pointer p = at_index(index);
		return *p; //reinterpret_cast<const_reference>(*p);
	}
	size_type size() const {
		return last_index;
	}
	size_type capacity()const {
		return num_pages() * page_size;
	}
	void reserve(size_type size) {
		if (size > last_index) {
			resize_pages(page_divide_ceil(size));
		}
	}
	//erase all elements, but dont deallocate anything
	void clear() {

		delete_range(0, last_index);
		last_index = 0;
	}
	void resize(size_type size) {
		auto old_size = last_index;

		resize_uninitialized(size);

		while (old_size < size) {
			init_at(old_size++);
		}
	}

	void resize(size_type size, const value_type& value) {

		auto old_size = last_index;

		resize_uninitialized(size);
		//default-initialize the elements properly if they arent trivial
		if (!std::is_trivially_destructible<T>::value) {
			int start = old_size;
			while (start < size) {
				init_at(start++);
			}
		}

		while (old_size < size) {
			operator[](old_size++) = value;
		}
	}

	template<typename V>
	void push_back(const V& value) {

		resize(last_index + 1, value);
	}
	template<typename V>
	void push_back(V&& value) {
		
		resize(last_index + 1, value);
		//resize_uninitialized(last_index + 1);
		back() = value;
	}
	template<typename ...Args> void emplace_back(Args&&... args)
	{
		const auto idx = last_index;
		resize_uninitialized(last_index + 1);

		init_at(idx, std::forward<Args>(args)...);
	}
	void pop_back() {
		resize(last_index - 1);
	}

	reference back() {
		return operator[](last_index - 1);
	}
private:

	void resize_uninitialized(size_type size) {


		if (size < last_index) {
			delete_range(last_index, size);
		}
		//if resizing to 0, this is a full clear
		if (size == 0) {
			resize_pages(0);
			delete[] page_array;
			page_capacity = 0;
			page_array = nullptr;
		}
		else {
			//resize_pages(page_divide_ceil(size));
			resize_pages(page_capacity*2);
		}

		last_index = size;
	}

	static size_t page_divide_ceil(size_t idx) {
		return (idx + page_size - 1) / page_size;
	}

	size_type num_pages() const {
		return last_index >> bits;
	}

	void delete_range(size_t start, size_t end) {
		//destroy all elements, only if they arent trivial
		if (!std::is_trivially_destructible<T>::value) {
			while (start < end) {
				at_index(start++)->~T();
			}
		}
	}

	pointer at_index(size_t index) const {
		const auto page_idx = index >> bits;
		const auto element_idx = index & mask;
		pointer tp = reinterpret_cast<pointer>(page_array[page_idx]);
		return tp + element_idx;
	}
	template<typename ...Args>
	void init_at(size_t index, Args&&... args) {
		new(at_index(index)) T(std::forward<Args>(args)...);
	}

	void create_page(size_t index) {
		//page_array[index] = malloc(sizeof(T) * page_size);
		page_array[index] = new page();
	}
	void destroy_page(size_t index) {
		//free(page_array[index]);
		delete page_array[index];
		page_array[index] = nullptr;
	}
	void resize_pages(size_type new_pages) {
		//no realloc needed
		if (new_pages < page_capacity) {
			//delete pages
			if (new_pages < num_pages()) {
				for (auto i = new_pages; i < num_pages(); i++) {
					destroy_page(i);
				}
			}
		}
		else if (new_pages > page_capacity) {
			if (page_capacity * 2 > new_pages)
			{
				new_pages = page_capacity * 2;
			}
			page** new_array = new page * [new_pages];

			if (page_array)
			{
				for (size_t i = 0; i < page_capacity; i++) {
					new_array[i] = page_array[i];
				}
				delete[] page_array;
			}

			page_array = new_array;
			for (size_t i = page_capacity; i < new_pages; i++) {
				create_page(i);
			}
			page_capacity = new_pages;
		}
	}


	size_type last_index{ 0 };
	page** page_array{ nullptr };

	//size of page array
	size_type page_capacity{ 0 };
};

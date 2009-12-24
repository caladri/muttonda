#ifndef	REF_H
#define	REF_H

template<typename T>
class Ref {
	class RefObj {
		T *ptr_;
		unsigned count_;
	public:
		RefObj(T *ptr)
		: ptr_(ptr),
		  count_(1)
		{ }

	private:
		~RefObj()
		{
			delete ptr_;
			ptr_ = NULL;
		}

	public:
		void hold(void)
		{
			count_++;
		}

		void drop(void)
		{
			if (count_-- == 1)
				delete this;
		}

		T *get(void) const
		{
			return (ptr_);
		}
	};

	RefObj *obj_;
public:
	Ref(T *ptr)
	: obj_(new RefObj(ptr))
	{ }

	Ref(const Ref& ref)
	: obj_(ref.obj_)
	{
		obj_->hold();
	}

	~Ref()
	{
		obj_->drop();
		obj_ = NULL;
	}

	const Ref& operator= (const Ref& ref)
	{
		obj_->drop();
		obj_ = ref.obj_;
		obj_->hold();
		return (*this);
	}

	T *operator-> (void) const
	{
		return (obj_->get());
	}

	const T& operator* (void) const
	{
		T *ptr = obj_->get();
		return (*ptr);
	}
};

#endif /* !REF_H */

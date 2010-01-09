#ifndef	REF_H
#define	REF_H

template<typename T>
class Ref {
	class RefObj {
		T *ptr_;
		unsigned count_;
		unsigned id_;
	public:
		RefObj(T *ptr)
		: ptr_(ptr),
		  count_(1),
		  id_(get_id())
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

		unsigned id(void) const
		{
			return (id_);
		}

	private:
		static unsigned get_id(void)
		{
			static unsigned last_id;
			return (++last_id);
		}
	};

	RefObj *obj_;
public:
	Ref(void)
	: obj_(NULL)
	{ }

	Ref(T *ptr)
	: obj_(new RefObj(ptr))
	{ }

	Ref(const Ref& ref)
	: obj_(ref.obj_)
	{
		if (obj_ != NULL)
			obj_->hold();
	}

	~Ref()
	{
		if (obj_ != NULL) {
			obj_->drop();
			obj_ = NULL;
		}
	}

	const Ref& operator= (const Ref& ref)
	{
		if (obj_ != NULL) {
			obj_->drop();
			obj_ = NULL;
		}

		if (ref.obj_ != NULL) {
			obj_ = ref.obj_;
			obj_->hold();
		}
		return (*this);
	}

	const T *operator-> (void) const
	{
		return (obj_->get());
	}

	const T *operator* (void) const
	{
		return (obj_->get());
	}

	bool null(void) const
	{
		return (obj_ == NULL);
	}

	unsigned id(void) const
	{
		if (obj_ == NULL)
			return (0);
		return (obj_->id());
	}
};

#endif /* !REF_H */

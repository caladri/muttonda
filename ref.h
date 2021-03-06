#ifndef	REF_H
#define	REF_H

class RefMeta {
protected:
	template<typename T>
	RefMeta(T *)
	{ }

public:
	virtual ~RefMeta()
	{ }
};

template<typename T, typename Tm = RefMeta *>
class Ref {
public:
	typedef	unsigned	id_t;

private:
	class RefObj {
		T *ptr_;
		Tm *meta_;
		unsigned count_;
		id_t id_;
	public:
		RefObj(T *ptr)
		: ptr_(ptr),
		  meta_(NULL),
		  count_(1),
		  id_(get_id())
		{ }

	private:
		~RefObj()
		{
			delete ptr_;
			ptr_ = NULL;

			if (meta_ != NULL) {
				delete meta_;
				meta_ = NULL;
			}
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

		Tm *meta(void)
		{
			if (meta_ == NULL)
				meta_ = new Tm(ptr_);
			return (meta_);
		}

		id_t id(void) const
		{
			return (id_);
		}

	private:
		static id_t get_id(void)
		{
			static id_t last_id;
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

	const T& operator* (void) const
	{
		const T *ptr = obj_->get();
		return (*ptr);
	}

	template<typename Tc>
	Tc cast(void) const
	{
		const T *ptr = obj_->get();
		return (dynamic_cast<Tc>(ptr));
	}

	bool null(void) const
	{
		return (obj_ == NULL);
	}

	Tm *meta(void) const
	{
		return (obj_->meta());
	}

	id_t id(void) const
	{
		if (obj_ == NULL)
			return (id_t(0));
		return (obj_->id());
	}

	bool operator< (const Ref& b) const
	{
		return (id() < b.id());
	}
};

#endif /* !REF_H */

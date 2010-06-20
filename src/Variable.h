// Variable.h
// Variables management

// RPN calculator

// Sébastien Millet
// August-December 2009

#ifndef VARIABLES_H
#define VARIABLES_H

#include "Common.h"
#include <string>
#include <vector>
#include <map>

class StackItem;

//
// Var
//

typedef enum {VAR_DIRECTORY, VAR_FILE} var_t;

class Var {

#ifdef DEBUG_CLASS_COUNT
	static long class_count;
#endif

	int order;
	std::map<std::string, Var*>::iterator* map_iterator_ptr;
	Var(const Var&);
	Var& operator=(const Var&);
public:
	Var(const int&);
	virtual ~Var() = 0;

#ifdef DEBUG_CLASS_COUNT
	static int get_class_count();
#endif

	virtual var_t get_type() const = 0;
	virtual int get_order() const;
	virtual void decrease_order();
	virtual void set_map_iterator(std::map<std::string, Var*>::iterator);
	virtual std::map<std::string, Var*>::iterator* get_map_iterator_ptr() const;
	virtual Var* dup(const bool&) const = 0;
	virtual void clear_si() = 0;
};


//
// VarFile
//

class VarFile : public Var {
	StackItem* si;
	VarFile& operator=(const VarFile&);
public:
	VarFile(const int&, StackItem* const = 0);
	virtual ~VarFile();
	virtual var_t get_type() const;
	void attach_si(StackItem*);
	virtual StackItem *get_si_dup() const;
	virtual const StackItem *get_const_si() const;
	virtual Var* dup(const bool&) const;
	virtual void clear_si();
};


//
// VarDirectory
//

class VarDirectory : public Var {
	std::map<std::string, Var*>* by_string;
	VarDirectory* parent;
	VarDirectory& operator=(const VarDirectory&);
public:
	VarDirectory(const int&, VarDirectory*);
	VarDirectory(const VarDirectory&, const bool&);
	virtual ~VarDirectory();
	virtual var_t get_type() const;
	virtual int new_vardirectory(const std::string&);
	virtual VarFile* new_varfile(const std::string&, const bool& = true);
	virtual st_err_t sto(const std::string&, StackItem*);
	virtual VarDirectory* get_parent() const;
	virtual Var* get_var(const std::string&) const;
	virtual st_err_t purge(const std::string&);
	virtual void get_vars(std::vector<std::string>*&) const;
	virtual Var* dup(const bool&) const;
	virtual size_t get_size() const;
	virtual void clear_si();
	virtual bool same(VarDirectory*) const;
};


//
// Tree
//

class Tree {

#ifdef DEBUG_CLASS_COUNT
	static long class_count;
#endif

	VarDirectory* root;
	VarDirectory* pwd;
	bool pwd_has_changed;
	virtual void set_pwd(VarDirectory* const);

public:
	Tree();
	virtual ~Tree();

#ifdef DEBUG_CLASS_COUNT
	static int get_class_count();
#endif

	virtual bool get_pwd_has_changed() const;
	virtual void reset_pwd_has_changed();
	virtual st_err_t crdir(const std::string&);
	virtual st_err_t sto(const std::string&, StackItem* const);
	virtual st_err_t purge(const std::string&);
	virtual st_err_t inner_rcl(const std::string&, Var*&);
	virtual st_err_t rcl(const std::string&, StackItem*&, Var*&);
	virtual st_err_t rcl_const(const std::string&, const StackItem*&, Var*&);
	virtual st_err_t rcl_for_eval(const std::string&, StackItem*&);
	virtual void get_var_list(std::vector<std::string>*&) const;
	virtual void get_path(std::vector<std::string>*&) const;
	virtual void home();
	virtual void up();
	virtual VarDirectory* save_pwd() const;
	virtual void recall_pwd(VarDirectory*);
};

#endif // VARIABLES_H


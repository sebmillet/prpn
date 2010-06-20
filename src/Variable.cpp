// Variable.cpp
// Variables management

// RPN calculator

// Sébastien Millet
// August-December 2009

#include "Common.h"
#include "Variable.h"
#include "Stack.h"

using namespace std;


//
// Var
//

#ifdef DEBUG_CLASS_COUNT
long Var::class_count = 0;
#endif

Var::Var(const int& o) : order(o), map_iterator_ptr(0) {
#ifdef DEBUG_CLASS_COUNT
		class_count++;
#endif
}

Var::~Var() {
	if (map_iterator_ptr != NULL)
		delete map_iterator_ptr;
#ifdef DEBUG_CLASS_COUNT
		class_count--;
#endif
}

#ifdef DEBUG_CLASS_COUNT
int Var::get_class_count() { return class_count; }
#endif

int Var::get_order() const { return order; }

void Var::decrease_order() { order--; }

void Var::set_map_iterator(map<string, Var*>::iterator it) {
	map_iterator_ptr = new map<string, Var*>::iterator(it);
}

map<string, Var*>::iterator* Var::get_map_iterator_ptr() const {
	return map_iterator_ptr;
}


//
// VarFile
//

VarFile::VarFile(const int& order, StackItem* const sisi) : Var(order), si(sisi) { }

VarFile::~VarFile() {
	if (si != NULL)
		delete si;
}

void VarFile::attach_si(StackItem* sisi) {
	if (si != NULL)
		delete si;
	si = sisi;
}

var_t VarFile::get_type() const { return VAR_FILE; }

StackItem* VarFile::get_si_dup() const {
	if (si != NULL)
		return si->dup();
	else
		return NULL;
}

const StackItem *VarFile::get_const_si() const {
	if (si != NULL)
		return const_cast<const StackItem*>(si);
	else
		return NULL;
}

Var* VarFile::dup(const bool& with_si) const {
	if (with_si) {
		StackItem* copy_si = si;
		if (copy_si != NULL)
			copy_si = copy_si->dup();
		return new VarFile(get_order(), copy_si);
	} else
		return new VarFile(get_order(), NULL);
}

void VarFile::clear_si() {
	attach_si(NULL);
}


//
// VarDirectory
//

VarDirectory::VarDirectory(const int& order, VarDirectory* p) : Var(order), by_string(new map<string, Var*>), parent(p) { }

VarDirectory::VarDirectory(const VarDirectory& vd, const bool& with_si) : Var(vd.get_order()), parent(vd.parent) {
	by_string = new map<string, Var*>;
	for (map<string, Var*>::iterator it = vd.by_string->begin(); it != vd.by_string->end(); it++)
		(*by_string)[it->first] = it->second->dup(with_si);
}

VarDirectory::~VarDirectory() {
	for (map<string, Var*>::iterator it = by_string->begin(); it != by_string->end(); it++) {
		delete it->second;
	}
	delete by_string;
}

var_t VarDirectory::get_type() const { return VAR_DIRECTORY; }

int VarDirectory::new_vardirectory(const string& s) {
	if (by_string->find(s) != by_string->end())
		return 1;
	size_t n = by_string->size() + 1;
	Var* e = new VarDirectory(n, this);
	(*by_string)[s] = e;
	e->set_map_iterator(by_string->find(s));
	return 0;
}

VarFile* VarDirectory::new_varfile(const string& s, const bool& allow_existing) {
	if (by_string->find(s) != by_string->end()) {
		Var* e = (*by_string)[s];
		if (e->get_type() == VAR_DIRECTORY)
			return 0;
		else if (allow_existing)
			return dynamic_cast<VarFile*>((*by_string)[s]);
		else
			return 0;
	}
	size_t n = by_string->size() + 1;
	VarFile* e = new VarFile(n);
	(*by_string)[s] = e;
	e->set_map_iterator(by_string->find(s));

	return e;
}

st_err_t VarDirectory::sto(const string& s, StackItem* si) {
	VarFile* e = new_varfile(s);
	if (e == 0)
		return ST_ERR_DIRECTORY_NOT_ALLOWED;
	e->attach_si(si);
	return ST_ERR_OK;
}

VarDirectory* VarDirectory::get_parent() const { return parent; }

Var* VarDirectory::get_var(const std::string& s) const {
	map<string, Var*>::const_iterator f = by_string->find(s);
	if (f != by_string->end())
		return f->second;
	return 0;
}

st_err_t VarDirectory::purge(const string& s) {
	map<string, Var*>::iterator f = by_string->find(s);
	if (f == by_string->end())
		return ST_ERR_OK;
	if (f->second->get_type() == VAR_DIRECTORY) {
		if (dynamic_cast<VarDirectory*>(f->second)->get_size() != 0)
			return ST_ERR_NON_EMPTY_DIRECTORY;
	}
	int o = f->second->get_order();
	delete f->second;
	by_string->erase(f);
	for (map<string, Var*>::iterator it = by_string->begin(); it != by_string->end(); it++) {
		if (it->second->get_order() > o)
			it->second->decrease_order();
	}
	return ST_ERR_OK;
}

void VarDirectory::get_vars(vector<string>*& by_order) const {
	int n = by_string->size();
	by_order = new vector<string>(n);
	for (map<string, Var*>::iterator it = by_string->begin(); it != by_string->end(); it++) {
		if (it->second->get_order() < 1 || it->second->get_order() > n)
			throw(CalcFatal(__FILE__, __LINE__, "VarDirectory::get_vars(): inconsistent variables ordering"));
		size_t pos = (size_t)(n - it->second->get_order());
		if ((*by_order)[pos] != "")
			throw(CalcFatal(__FILE__, __LINE__, "VarDirectory::get_vars(): multiple entries have the same order"));
		(*by_order)[pos] = it->first;
	}
}

Var* VarDirectory::dup(const bool& with_si) const {
	return new VarDirectory(*this, with_si);
}

size_t VarDirectory::get_size() const {
	return by_string->size();
}

void VarDirectory::clear_si() {
	for (map<string, Var*>::iterator it = by_string->begin(); it != by_string->end(); it++)
		it->second->clear_si();
}

bool VarDirectory::same(VarDirectory* pv) const {
	bool r = false;
	vector<string>* bo;
	get_vars(bo);
	vector<string>* pv_bo;
	pv->get_vars(pv_bo);
	if (bo->size() == pv_bo->size()) {
		vector<string>::iterator it_pv_bo = pv_bo->begin();
		r = true;
		for (vector<string>::iterator it_bo = bo->begin(); it_bo != bo->end(); it_bo++) {
			if (*it_bo != *it_pv_bo) {
				r = false;
				break;
			}
			it_pv_bo++;
		}
	}
	delete bo;
	delete pv_bo;
	return r;
}

//
// Tree
//

#ifdef DEBUG_CLASS_COUNT
long Tree::class_count = 0;
#endif

Tree::Tree() : root(new VarDirectory(0, 0)), pwd_has_changed(true) {

#ifdef DEBUG_CLASS_COUNT
		class_count++;
#endif

	  // The initialization 'pwd_has_changed(true)' above is useless due to the instruction below,
	  // nevertheless I find it more resilient with it. So what?
	set_pwd(root);
}

Tree::~Tree() {

#ifdef DEBUG_CLASS_COUNT
		class_count--;
#endif

	delete root;
}

#ifdef DEBUG_CLASS_COUNT
int Tree::get_class_count() { return class_count; }
#endif

void Tree::set_pwd(VarDirectory* const p) {
	pwd = p;
	pwd_has_changed = true;
}

bool Tree::get_pwd_has_changed() const { return pwd_has_changed; }

void Tree::reset_pwd_has_changed() { pwd_has_changed = false; }

st_err_t Tree::crdir(const std::string& s) {
	if (pwd->new_vardirectory(s) != 0)
		return ST_ERR_DIRECTORY_NOT_ALLOWED;
	return ST_ERR_OK;
}

st_err_t Tree::sto(const string& s, StackItem* const si) { return pwd->sto(s, si); }

st_err_t Tree::purge(const string& s) { return pwd->purge(s); }

st_err_t Tree::inner_rcl(const string& s, Var*& e) {
	VarDirectory* p = pwd;
	do {
		e = p->get_var(s);
		if (e == 0)
			p = p->get_parent();
	} while (e == 0 && p != 0);
	if (e == 0)
		return ST_ERR_UNDEFINED_NAME;
	if (e->get_type() == VAR_DIRECTORY)
		return ST_ERR_DIRECTORY_NOT_ALLOWED;
	return ST_ERR_OK;
}

st_err_t Tree::rcl(const string& s, StackItem*& si, Var*& e) {
	st_err_t c = inner_rcl(s, e);
	if (c != ST_ERR_OK)
		return c;
	si = dynamic_cast<VarFile*>(e)->get_si_dup();
	return ST_ERR_OK;
}

st_err_t Tree::rcl_const(const string& s, const StackItem*& si, Var*& e) {
	st_err_t c = inner_rcl(s, e);
	if (c != ST_ERR_OK)
		return c;
	si = dynamic_cast<VarFile*>(e)->get_const_si();
	return ST_ERR_OK;
}

st_err_t Tree::rcl_for_eval(const string& s, StackItem*& si) {
	Var* e;
	st_err_t c = rcl(s, si, e);
	if (c == ST_ERR_DIRECTORY_NOT_ALLOWED) {
		set_pwd(dynamic_cast<VarDirectory*>(e));
		si = 0;
		c = ST_ERR_OK;
	}
	return c;
}

void Tree::get_var_list(vector<string>*& by_order) const { pwd->get_vars(by_order); }

void Tree::get_path(vector<string>*& directories) const {
	VarDirectory* p = pwd;
	size_t n = 0;
	do {
		n++;
		p = p->get_parent();
	} while (p != 0);
	p = pwd;
	directories = new vector<string>(n);
	std::map<std::string, Var*>::iterator* pit;
	while (n != 0) {
		pit = p->get_map_iterator_ptr();
		if (pit == 0)
			(*directories)[--n] = "HOME";
		else
			(*directories)[--n] = (*pit)->first;
		p = p->get_parent();
	}
	if (p != 0)
		throw(CalcFatal(__FILE__, __LINE__, "Tree:get_path(): Houston, on a un problème !"));
}

void Tree::home() {
	set_pwd(root);
}

void Tree::up() {
	if (pwd->get_parent() != 0)
		set_pwd(pwd->get_parent());
}

VarDirectory* Tree::save_pwd() const { return pwd; }
void Tree::recall_pwd(VarDirectory* d) { pwd = d; }


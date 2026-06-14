#ifndef 	VSERVER_HPP
# define	VSERVER_HPP

# include "conf.hpp"
# include "http/HTTP_Request.hpp"
# include <string>



typedef struct
{
	std::string					query;
	std::string					resolved_path;

	std::map<int, std::string>	errors;
	std::set<std::string>		limit_except;
	std::string					file_succes_download;
	std::string					file_succes_delete;
	std::string					file_tmp_css;

	bool						directory;

	std::string					root;
	std::vector<std::string>	index;
	bool						autoindex;

	std::set<std::string>		server_name;
	std::string					server_port;
	bool						cgi;
	std::string					cgi_extension;
	std::string 				cgi_path;

	HTTP_Request*				request;
}	request_context_t;



class VServer
{
	private:
		const vserver_t		_vserver;
		request_context_t	_context;

		void				clear_context();
		void				construct_request_path();
		const location_t*	find_location()										const;
		void				build_context(const location_t* location);
		void				verif_body_size()									const;

	public:
		VServer(const vserver_t& server);
		~VServer();

		const request_context_t		execute(const HTTP_Request& request);
		bool						is_it_me(const std::string& name, int port)	const;
		bool						is_port(int port)							const;

		void						print_context() const;
};

#endif
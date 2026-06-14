# include "handlers/CGIHandler.hpp"
# include "tools/char_tools.hpp"
# include <cctype>
# include <unistd.h>
# include "tools/getTimeNow.hpp"

# include <vector>

CGIHandler::CGIHandler()
{

}

CGIHandler::~CGIHandler()
{

}

static std::string	creat_headers_cgi(const std::string& header)
{
	std::string	cgi_header = "HTTP_";

	for (size_t i = 0; i < header.size(); i++)
	{
		if (header[i] == '-')
		{
			cgi_header.push_back('_');
			continue;
		}
		cgi_header.push_back(std::toupper(header[i]));
	}
	cgi_header.push_back('=');
	return cgi_header;
}

static std::vector<char*>	build_env(request_context_t& context)
{
	std::string	uri = context.resolved_path;
	uri.erase(0, context.root.size());

	std::vector<std::string>	env;
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("REQUEST_METHOD=" + context.request->start_line.method);
	env.push_back("SERVER_PROTOCOL=" + context.request->start_line.version.name + '/'
	+ char_tools::itostr(context.request->start_line.version.minor) + '.' + char_tools::itostr(context.request->start_line.version.major));
	env.push_back("SCRIPT_NAME=" + uri);
	env.push_back("QUERY_STRING=" + context.query);
	env.push_back("SERVER_NAME=" + *context.server_name.begin());
	env.push_back("SERVER_PORT=" + context.server_port);

	if (context.request->entity_body.size != 0)
	{
		env.push_back("CONTENT_TYPE=" + context.request->headers.find("content-type")->second);
		env.push_back("CONTENT_LENGTH=" + char_tools::itostr(context.request->entity_body.size));
	}

	std::map<std::string, std::string>::const_iterator	cit = context.request->headers.begin();
	std::map<std::string, std::string>::const_iterator	cite = context.request->headers.end();

	for (; cit != cite; ++cit)
	{
		for (size_t i = 0; i < sizeof(headers_blacklist) / sizeof(char*); i++)
			if (cit->first == headers_blacklist[i])
				continue;

		env.push_back(creat_headers_cgi(cit->first) + cit->second);
	}

	{
		if (context.request->session) {
			SessionManager::session_data_t::iterator	it = context.request->session->values.find("login");
			if (it != context.request->session->values.end()) {
				env.push_back("USER_NAME=" + it->second);
			}
			else {
				env.push_back("USER_NAME=");
			}
		}
		else {
			env.push_back("USER_NAME=");
		}
	}

	std::vector<char*>	envp;
	for (size_t i = 0; i < env.size(); ++i)
	{
		char* tmp = new char[env[i].size() + 1];
		std::copy(env[i].c_str(), env[i].c_str() + env[i].size() + 1, tmp);
		envp.push_back(tmp);
	}
	envp.push_back(NULL);
	
	
	return envp;
}

static std::vector<char*>	build_argv(request_context_t& context)
{
	std::vector<std::string>	argv_construction;
	
	if (context.cgi_path.empty() == false)
	{
		argv_construction.push_back(context.cgi_path);
	}
	
	argv_construction.push_back(context.resolved_path);
	
	std::vector<char*>	argv;
	for (size_t i = 0; i < argv_construction.size(); ++i)
	{
		char* tmp = new char[argv_construction[i].size() + 1];
		std::copy(argv_construction[i].c_str(), argv_construction[i].c_str() + argv_construction[i].size() + 1, tmp);
		argv.push_back(tmp);
	}
	argv.push_back(NULL);

return argv;
}

static void	process_child(request_context_t& context, int pipe_in[2], int pipe_out[2])
{
	std::vector<char*>	envp = build_env(context);
	std::vector<char*>	argv = build_argv(context);

	GarbageCollector::get_instance().clean_all();

	close(pipe_in[1]);
	close(pipe_out[0]);

	if (dup2(pipe_in[0], STDIN_FILENO) == -1)
		_exit(1);
	if (dup2(pipe_out[1], STDOUT_FILENO) == -1)
		_exit(1);

	close(pipe_in[0]);
	close(pipe_out[1]);

	if (!context.cgi_path.empty())
		execve(context.cgi_path.c_str(), &argv[0], &envp[0]);
	else
		execve(context.resolved_path.c_str(), &argv[0], &envp[0]);

	for (size_t i = 0; i < envp.size(); ++i)
		delete[] envp[i];
	for (size_t i = 0; i < argv.size(); ++i)
		delete[] argv[i];

	_exit(1);
}

resource_context_t	CGIHandler::execute(request_context_t& context)
{
	resource_context_t	resource;
	int					pipe_in[2], pipe_out[2];
	pid_t				pid;
	
	if (pipe(pipe_in) == -1)
		throw std::runtime_error("error pipe creation");
	if (pipe(pipe_out) == -1)
		throw std::runtime_error("error pipe creation");

	pid = fork();
	if (pid == -1)
		throw std::runtime_error("error fork creation");

	if (pid == 0)
	{
		process_child(context, pipe_in, pipe_out);
		_exit(1);
	}

	{
		fcntl(pipe_out[0], F_SETFL, O_NONBLOCK);
	}

	close(pipe_in[0]);
	close(pipe_out[1]);
	resource.file_cgi_write.fd = pipe_in[1];
	resource.file_cgi_write.size = context.request->entity_body.size;
	resource.file_cgi_read.fd = pipe_out[0];
	resource.file_cgi_read.size = 0;
	resource.cgi.process = pid;
	resource.cgi.start_cgi = getTimeNow();

	resource.http.status = E_HTTP_OK;
	resource.http.reason = const_cast<char*>(http_error_reason(E_HTTP_OK));

	return resource;
}

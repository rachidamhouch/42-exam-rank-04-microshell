#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int print_error(char *msg, char *arg)
{
	while (*msg)
		write(2, msg++, 1);
	if (arg)
		while (*arg)
			write(2, arg++, 1);
	return (write(2, "\n", 1));
}
int execute(char **av, char **env, int i)
{
	av[i] = NULL;
	return (execve(av[0], av, env) && print_error("error: cannot execute ", av[0]));
}
int main(int ac, char **av, char **env)
{
	int i = 0, pip[2];

	ac = dup(0);
	while (av[i] && av[i + 1])
	{
		av = av + i + 1;
		i = 0;
		while (av[i] && strcmp(av[i], ";") && strcmp(av[i], "|"))
			i++;
		if (av[0] && !strcmp(av[0], "cd"))
		{
			if (i != 2)
				print_error("error: cd: bad arguments", NULL);
			else if (chdir(av[1]))
				print_error("error: cd: cannot change directory to ", av[1]);
		}
		else if(i != 0 && (!av[i] || !strcmp(av[i], ";")))
		{
			if (!fork())
			{
				if (execute(av, env, i))
					return (1);
			}
			else
			{
				dup2(ac, 0);
				while (waitpid(-1, 0, 0) != -1);
			}
		}
		else if(i != 0 && !strcmp(av[i], "|"))
		{
			pipe(pip);
			if (!fork())
			{
				dup2(pip[1], 1);
				close(pip[0]);
				close(pip[1]);
				if (execute(av, env, i))
					return (1);
			}
			else
			{
				dup2(pip[0], 0);
				close(pip[0]);
				close(pip[1]);
			}
		}
	}
	return (close(ac));
}

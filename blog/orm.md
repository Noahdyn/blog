---
title: Fixing the most common ORM pitfall - The n+1 problem
date: 2026-05-17
header_img_src: path_finding_asset_1.svg
---

# Introduction

ORM Tools, such as the Entity Framework are a very popular abstraction in the enterprise world. Instead of having to think about your database layout and handling the mapping between database tables and application objects you can just declare the models you want to use and have your ORM handle the rest for you.

However this disconnect between the developer and the underlying database technology frequently results in inefficient application code, the one example I've seen most frequently in real projects is the so called n+1 problem. In fact I've seen it so many times in different companies that I've decided to write this blog post, offering a simple fix.

# The Problem

But first we have to understand the problem at hand. So to visualize we're going to look at an example in the Entity Framework:

```cs
public class Customer
{
    public int Id { get; set; }
    public string Name { get; set; } = "";
    public List<Order> Orders { get; set; } = new();
}

public class Order
{
    public int Id { get; set; }
    public string Product { get; set; } = "";
    public int CustomerId { get; set; }
    public Customer Customer { get; set; } = null!;
}

public class AppDbContext : DbContext
{
    public DbSet<Customer> Customers => Set<Customer>();
    public DbSet<Order> Orders => Set<Order>();

    protected override void OnConfiguring(DbContextOptionsBuilder options)
    {
        options
            .UseSqlite("Data Source=demo.db")
            .LogTo(Console.WriteLine, Microsoft.Extensions.Logging.LogLevel.Information)
            .EnableSensitiveDataLogging();
    }
}
```

Here we are indirectly creating two db tables, a Customer table and an Order table, sharing a one-to-many relation with each other.

Now in your application you may end up iterating over all orders and do something like this:

```cs
var orders = await context.Orders.ToListAsync();

foreach (var order in orders)
{
    var customerName = context.Customers
        .Where(c => c.Id == order.CustomerId)
        .Select(c => c.Name)
        .First();
    // send out invoice..
}
```

Now this short snipped might seem innocent, but actually carries a massive performance liability to your application. The way Entity Framework (or any ORM really) interprets this is to send a query to your database on every loop iteration, fetching the given customer name from the Customer table, resulting in one query for each order plus the original orders query, giving this problem its name, the n+1 problem.

If we look at the SQL produced by this loop, we can see the following queries:

```sql
-- initial orders query
SELECT "o"."Id", "o"."CustomerId", "o"."Product"
      FROM "Orders" AS "o"

-- generated for each iteration of the loop
 SELECT "c"."Name"
      FROM "Customers" AS "c"
      WHERE "c"."Id" = @__order_CustomerId_0
      LIMIT 1
```

This is a massive liability in regards to our applications performance, hitting the database and thus on-disk memory n+1 times is insanely inefficient.

# The Solution

So how do we actually fix this. Those of you who remember their days on university, or had the joy of working directly with a database rather than through an ORM may remember these things called JOINS.

A join merges two database tables and allows you to view desired columns from both tables in one result (using only 1 query). 

Luckily any reasonable ORM lets you do this in your application code aswell:

```cs

var ordersWithCustomers = await context.Orders
    .Include(o => o.Customer)
    .ToListAsync();

foreach (var order in ordersWithCustomers)
{
    var customerName = order.Customer.Name;
    // send out invoice..
}

```

This way we fetch the required information directly with our initial query and can use it in our application without requiring additional database queries.

Lets have a look at the generated SQL:

```sql
SELECT "o"."Id", "o"."CustomerId", "o"."Product", "c"."Id", "c"."Name"
    FROM "Orders" AS "o"
    INNER JOIN "Customers" AS "c" ON "o"."CustomerId" = "c"."Id"
```

Here we can see Entity Framework performing a single query, joining both tables together and thus greatly increasing our applications performance.




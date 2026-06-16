import { sql } from "@vercel/postgres";

let initialized = false;

export async function ensureDb() {
  if (initialized) return;

  await sql`
    CREATE TABLE IF NOT EXISTS users (
      id SERIAL PRIMARY KEY,
      username VARCHAR(64) UNIQUE NOT NULL,
      email VARCHAR(255) NOT NULL,
      password_hash TEXT NOT NULL,
      app_name VARCHAR(128) NOT NULL,
      owner_id VARCHAR(32) UNIQUE NOT NULL,
      app_secret VARCHAR(64) NOT NULL,
      app_version VARCHAR(16) DEFAULT '1.0',
      created_at TIMESTAMPTZ DEFAULT NOW()
    )
  `;

  await sql`
    CREATE TABLE IF NOT EXISTS licenses (
      id VARCHAR(32) PRIMARY KEY,
      user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
      license_key VARCHAR(128) NOT NULL,
      status VARCHAR(16) DEFAULT 'unused',
      level INTEGER DEFAULT 1,
      expiry_unit VARCHAR(16),
      expiry_duration INTEGER,
      expiry_label VARCHAR(64),
      note TEXT DEFAULT '',
      created_at TIMESTAMPTZ DEFAULT NOW()
    )
  `;

  await sql`CREATE INDEX IF NOT EXISTS idx_licenses_user_id ON licenses(user_id)`;
  await sql`CREATE INDEX IF NOT EXISTS idx_licenses_key ON licenses(license_key)`;

  initialized = true;
}

export { sql };
